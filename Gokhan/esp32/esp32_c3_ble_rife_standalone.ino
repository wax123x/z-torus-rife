/*
 * =====================================================
 *  Z-Torus ESP32-C3 SuperMini BLE Standalone Firmware v2.2
 * =====================================================
 *  Pin Bağlantıları:
 *    - GPIO 4  ➔ MOSFET Modülü Sinyal Girişi (PWM)
 * =====================================================
 *  Desteklenen BLE Komutları (Web/App Arayüzünden Gelen):
 *    - f528,5   ➔ 528 Hz frekans, 5 dakika süre
 *    - Q_CLR    ➔ Frekans kuyruğunu temizle
 *    - Q_ADD:hz,min ➔ Kuyruğa frekans ve dakika ekle
 *    - Q_START  ➔ Kuyruktaki frekansları otonom yürüt
 *    - Q_LOOP:1 ➔ Sürekli döngüyü aç/kapat
 *    - w1       ➔ Kare Dalga Modu (%50 Duty)
 *    - w2       ➔ Pulse Modu (%20 Duty)
 *    - STOP     ➔ Çıkışı ve kuyruğu tamamen durdur
 *    - STATUS   ➔ Durum sorgulama
 * =====================================================
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>
#include "esp_timer.h"
#include "driver/gpio.h"

#define HBRIDGE_IN1_PIN 4   // ESP32-C3 SuperMini DRV8871 IN1 Çıkışı (GPIO 4 - Faz A)
#define HBRIDGE_IN2_PIN 3   // ESP32-C3 SuperMini DRV8871 IN2 Çıkışı (GPIO 3 - Faz B - Zıt Faz Biphasic AC)
#define AUDIO_PIN       1   // ESP32-C3 SuperMini Kulaklık Sinyal Çıkışı (GPIO 1)
#define BUZZER_PIN      5   // ESP32-C3 SuperMini Buzzer Çıkışı (GPIO 5)
#define BATTERY_ADC_PIN 0  // ESP32-C3 GPIO 0 (2S Pil Voltaj Bölücü ADC Pini)
#define MAX_QUEUE_SIZE 100 // Maksimum frekans sırası kapasitesi

// ── 2S Pil Yüzdesi Okuma Fonksiyonu (16-Örnekli Filtreli & Rock-Solid Kararlı Gösterge) ──
int readBatteryPercentage() {
    long sumADC = 0;
    for (int i = 0; i < 16; i++) {
        sumADC += analogRead(BATTERY_ADC_PIN);
        delayMicroseconds(100);
    }
    float rawADC = sumADC / 16.0f;
    float adcVoltage = (rawADC / 4095.0f) * 3.3f;
    float batteryVoltage = adcVoltage * 8.75f; // Tam nokta atışı 7.37V pilde %57 gösterim kalibrasyonu

    int percentage = (int)(((batteryVoltage - 6.0f) / (8.4f - 6.0f)) * 100.0f);
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    return percentage;
}

unsigned long lastBatteryNotifyTime = 0;

// ── Z-Torus Evrensel Buzzer Sürücüsü (Aktif & Pasif Buzzer Uyumlu Yüksek Sesli Bip) ──
void playBuzzerTone(uint32_t freq, uint32_t durMs) {
    if (freq == 0) {
        digitalWrite(BUZZER_PIN, LOW);
        delay(durMs);
        return;
    }
    pinMode(BUZZER_PIN, OUTPUT);
    unsigned long startMs = millis();
    unsigned long halfPeriodUs = 1000000UL / (freq * 2);
    while (millis() - startMs < durMs) {
        digitalWrite(BUZZER_PIN, HIGH);
        delayMicroseconds(halfPeriodUs);
        digitalWrite(BUZZER_PIN, LOW);
        delayMicroseconds(halfPeriodUs);
    }
    digitalWrite(BUZZER_PIN, LOW);
}

void playBuzzerFinishChime() {
    playBuzzerTone(1047, 100); // C6
    delay(50);
    playBuzzerTone(1318, 100); // E6
    delay(50);
    playBuzzerTone(1568, 150); // G6
    delay(50);
    playBuzzerTone(2093, 300); // C7 (Bitiş)
    digitalWrite(BUZZER_PIN, LOW);
}

// ─── BLE UUID'ler (Z-Torus Uygulama Uyumlu) ─────────
#define SERVICE_UUID     "12340000-0000-1000-8000-00805f9b34fb"
#define CHAR_WRITE_UUID  "12340001-0000-1000-8000-00805f9b34fb"
#define CHAR_NOTIFY_UUID "12340002-0000-1000-8000-00805f9b34fb"

// ─── Global Değişkenler ────────────────────────────
BLEServer*         pServer            = NULL;
BLECharacteristic* pWriteChar         = NULL;
BLECharacteristic* pNotifyChar        = NULL;

bool               deviceConnected    = false;
bool               oldDeviceConnected = false;

// ── Frekans Üretim Değişkenleri ─────────────────────
volatile bool      isPlaying          = false;
volatile float     currentFreqHz      = 0.0;
unsigned long      playStartTime      = 0;
unsigned long      playDurationMs     = 0;
int                waveMode           = 1; // 1 = Kare (%50), 2 = Pulse (%20)

// ── Otonom Frekans Kuyruğu (Queue) ──────────────────
struct FreqItem {
    float         hz;
    unsigned long durationMs;
};

FreqItem           freqQueue[MAX_QUEUE_SIZE];
int                queueCount         = 0;
int                currentQueueIdx    = 0;
bool               isQueueRunning     = false;
bool               isLoopEnabled      = false;

// ─── Kalıcı Hafıza ─────────────────────────────────
Preferences prefs;
String      cihazIsmi;

// ─── esp_timer (Düşük/Orta Hz için ultra-hassas yazılımsal timer) ──
esp_timer_handle_t  squareTimer = NULL;
volatile bool       pinState    = false;
uint64_t            timerOnUs   = 0;
uint64_t            timerOffUs  = 0;

// Timer Callback (Düşük ve Orta frekanslar için Çift Fazlı Biphasic AC H-Köprüsü Timer)
void IRAM_ATTR timerCallback(void* arg) {
    pinState = !pinState;
    gpio_set_level((gpio_num_t)HBRIDGE_IN1_PIN, pinState ? 1 : 0);
    gpio_set_level((gpio_num_t)HBRIDGE_IN2_PIN, pinState ? 0 : 1); // Zıt Faz Biphasic AC (+40V / -40V)
    gpio_set_level((gpio_num_t)AUDIO_PIN, pinState ? 1 : 0);

    if (squareTimer != NULL && isPlaying && currentFreqHz < 1000.0f) {
        uint64_t nextDelay = pinState ? timerOnUs : timerOffUs;
        esp_timer_start_once(squareTimer, nextDelay);
    }
}

void stopSoftwareTimer() {
    if (squareTimer != NULL) {
        esp_timer_stop(squareTimer);
        esp_timer_delete(squareTimer);
        squareTimer = NULL;
    }
}

void stopLedc() {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
    ledcDetach(HBRIDGE_IN1_PIN);
    ledcDetach(HBRIDGE_IN2_PIN);
    ledcDetach(AUDIO_PIN);
#else
    ledcDetachPin(HBRIDGE_IN1_PIN);
    ledcDetachPin(HBRIDGE_IN2_PIN);
    ledcDetachPin(AUDIO_PIN);
#endif
    gpio_reset_pin((gpio_num_t)HBRIDGE_IN1_PIN);
    gpio_reset_pin((gpio_num_t)HBRIDGE_IN2_PIN);
    gpio_reset_pin((gpio_num_t)AUDIO_PIN);
}

void stopFreq() {
    isPlaying      = false;
    currentFreqHz  = 0.0;
    playDurationMs = 0;
    
    stopSoftwareTimer();
    stopLedc();

    pinMode(HBRIDGE_IN1_PIN, OUTPUT);
    digitalWrite(HBRIDGE_IN1_PIN, LOW);
    gpio_set_pull_mode((gpio_num_t)HBRIDGE_IN1_PIN, GPIO_PULLDOWN_ONLY);
    
    pinMode(HBRIDGE_IN2_PIN, OUTPUT);
    digitalWrite(HBRIDGE_IN2_PIN, LOW);
    gpio_set_pull_mode((gpio_num_t)HBRIDGE_IN2_PIN, GPIO_PULLDOWN_ONLY);
    
    pinMode(AUDIO_PIN, OUTPUT);
    digitalWrite(AUDIO_PIN, LOW);
    
    pinState = false;
    Serial.println("[Sistem] Frekans Durduruldu (Tam Sessiz Biphasic AC H-Köprüsü).");
}

void stopAllAndQueue() {
    isQueueRunning = false;
    queueCount     = 0;
    currentQueueIdx= 0;
    stopFreq();
}

void sendNotify(String msg);

void startFreq(float hz) {
    if (hz <= 0.0f) {
        stopFreq();
        return;
    }

    stopSoftwareTimer();
    stopLedc();

    currentFreqHz = hz;
    isPlaying     = true;

    // ── Düşük ve Orta Frekanslar (< 1000 Hz, Örn: 130 Hz, 528 Hz): esp_timer ──
    if (hz < 1000.0f) {
        uint64_t periodUs = (uint64_t)(1000000.0f / hz);

        if (waveMode == 2) {
            // Pulse Modu (%20 Açık, %80 Kapalı)
            timerOnUs  = (uint64_t)(periodUs * 0.20f);
            if (timerOnUs < 10) timerOnUs = 10;
            timerOffUs = periodUs - timerOnUs;
        } else {
            // Kare Dalga (%50 Açık, %50 Kapalı)
            timerOnUs  = periodUs / 2;
            timerOffUs = periodUs - timerOnUs;
        }

        gpio_config_t io_conf = {};
        io_conf.intr_type    = GPIO_INTR_DISABLE;
        io_conf.mode         = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << HBRIDGE_IN1_PIN) | (1ULL << HBRIDGE_IN2_PIN) | (1ULL << AUDIO_PIN);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
        gpio_config(&io_conf);

        esp_timer_create_args_t timerArgs = {};
        timerArgs.callback        = timerCallback;
        timerArgs.arg             = NULL;
        timerArgs.dispatch_method = ESP_TIMER_TASK;
        timerArgs.name            = "sq_wave";

        esp_timer_create(&timerArgs, &squareTimer);

        pinState = true;
        gpio_set_level((gpio_num_t)HBRIDGE_IN1_PIN, 1);
        gpio_set_level((gpio_num_t)HBRIDGE_IN2_PIN, 0); // Zıt Faz Biphasic AC
        gpio_set_level((gpio_num_t)AUDIO_PIN, 1);
        esp_timer_start_once(squareTimer, timerOnUs);

        Serial.printf("[Sistem] Çift Fazlı Biphasic AC H-Köprüsü Başlatıldı (IN1:4, IN2:3, AUDIO:1): %.2f Hz (Mod: %s)\n", hz, (waveMode == 2 ? "PULSE %20" : "KARE %50"));
    } 
    // ── Yüksek Frekanslar (>= 1000 Hz): LEDC PWM (Sabit 8-Bit Ultra-Kararlı Mod) ──
    else {
        gpio_reset_pin((gpio_num_t)HBRIDGE_IN1_PIN);
        gpio_reset_pin((gpio_num_t)HBRIDGE_IN2_PIN);
        gpio_reset_pin((gpio_num_t)AUDIO_PIN);
        pinMode(HBRIDGE_IN1_PIN, OUTPUT);
        pinMode(HBRIDGE_IN2_PIN, OUTPUT);
        pinMode(AUDIO_PIN, OUTPUT);

        int bits = 8; // Sabit 8-Bit (256 Adım) - Tüm frekanslarda maksimum kararlılık ve sıfır zamanlayıcı sıçraması

        uint32_t maxDuty     = (1 << bits);
        uint32_t duty        = (waveMode == 2) ? (uint32_t)(maxDuty * 0.20f) : (uint32_t)(maxDuty * 0.50f);
        uint32_t freqInt     = (uint32_t)round(hz);

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
        ledcAttach(HBRIDGE_IN1_PIN, freqInt, bits);
        ledcWrite(HBRIDGE_IN1_PIN, duty);

        ledcAttach(HBRIDGE_IN2_PIN, freqInt, bits);
        ledcWrite(HBRIDGE_IN2_PIN, maxDuty - duty); // Inverted Phase B

        ledcAttach(AUDIO_PIN, freqInt, bits);
        ledcWrite(AUDIO_PIN, duty);
#else
        ledcSetup(0, freqInt, bits);
        ledcAttachPin(HBRIDGE_IN1_PIN, 0);
        ledcWrite(0, duty);

        ledcSetup(1, freqInt, bits);
        ledcAttachPin(HBRIDGE_IN2_PIN, 1);
        ledcWrite(1, maxDuty - duty); // Inverted Phase B

        ledcAttachPin(AUDIO_PIN, 0);
#endif
        Serial.printf("[Sistem] LEDC PWM Biphasic AC Başlatıldı (IN1:4, IN2:3, AUDIO:1): %.2f Hz\n", hz);
    }
}

// ── Kuyruktaki Bir Sonraki Frekansa Geç ──
void playNextInQueue() {
    if (queueCount == 0 || currentQueueIdx >= queueCount) {
        if (isLoopEnabled && queueCount > 0) {
            currentQueueIdx = 0; // Başa dön
        } else {
            stopAllAndQueue();
            sendNotify("FIN");
            playBuzzerFinishChime(); // ESP32 Donanımsal Buzzer Bitiş Uyarısı
            Serial.println("[Kuyruk] Tüm frekanslar tamamlandı -> FIN (Buzzer Çaldı)");
            return;
        }
    }

    FreqItem item = freqQueue[currentQueueIdx];
    startFreq(item.hz);
    playStartTime  = millis();
    playDurationMs = item.durationMs;

    String notifyStr = "Q_NEXT:" + String(currentQueueIdx) + ":" + String(item.hz, 2);
    sendNotify(notifyStr);
    Serial.printf("[Kuyruk %d/%d] %.2f Hz başlatıldı (%ld ms)\n", currentQueueIdx + 1, queueCount, item.hz, item.durationMs);

    currentQueueIdx++;
}

// ─── BLE Notify Gönder (ESP32 → Uygulama) ──────────
void sendNotify(String msg) {
    if (deviceConnected && pNotifyChar) {
        pNotifyChar->setValue(msg.c_str());
        pNotifyChar->notify();
        Serial.printf("[BLE→App] %s\n", msg.c_str());
    }
}

// ─── BLE Sunucu Callbacks ──────────────────────────
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
        Serial.println("[BLE] Bağlantı kuruldu!");
    }
    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
        Serial.println("[BLE] Bağlantı kesildi. (Otonom Kuyruk Devam Ediyor)");
    }
};

// ─── BLE Write Callback (Uygulamadan Komut Geldi) ──
class MyWriteCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
        String komut = pChar->getValue();
        komut.trim();
        if (komut.length() == 0) return;
        Serial.printf("[BLE←App] %s\n", komut.c_str());

        // ── 1. KUYRUK KONTROL KOMUTLARI ────────────
        if (komut.equalsIgnoreCase("Q_CLR")) {
            stopAllAndQueue();
            sendNotify("Q_CLR_OK");
        }
        else if (komut.startsWith("Q_LOOP:")) {
            isLoopEnabled = (komut.substring(7).toInt() == 1);
            sendNotify("Q_LOOP_OK");
        }
        else if (komut.startsWith("Q_ADD:")) {
            String params = komut.substring(6);
            int commaPos = params.lastIndexOf(',');
            if (commaPos > 0 && queueCount < MAX_QUEUE_SIZE) {
                String strHz = params.substring(0, commaPos);
                String strMin = params.substring(commaPos + 1);
                strHz.replace(',', '.');
                strMin.replace(',', '.');
                float hz  = strHz.toFloat();
                float min = strMin.toFloat();
                if (hz > 0.0f) {
                    freqQueue[queueCount].hz = hz;
                    freqQueue[queueCount].durationMs = (min > 0.0f) ? ((unsigned long)(min * 60000.0f)) : 300000UL; // Varsayılan 5 dk
                    queueCount++;
                    sendNotify("Q_ADD_OK:" + String(queueCount));
                }
            }
        }
        else if (komut.equalsIgnoreCase("Q_START")) {
            if (queueCount > 0) {
                currentQueueIdx = 0;
                isQueueRunning  = true;
                playNextInQueue();
            }
        }

        // ── 2. DALGA ŞEKLİ KOMUTLARI ──────────────
        else if (komut.equalsIgnoreCase("w0") || komut.equalsIgnoreCase("w1")) {
            waveMode = 1; // Kare Dalga (%50 Duty)
            sendNotify("MODE:SQUARE");
            if (isPlaying && currentFreqHz > 0) startFreq(currentFreqHz);
        }
        else if (komut.equalsIgnoreCase("w2")) {
            waveMode = 2; // Pulse Modu (%20 Duty)
            sendNotify("MODE:PULSE");
            if (isPlaying && currentFreqHz > 0) startFreq(currentFreqHz);
        }

        // ── 3. TEKİL FREKANS KOMUTU (f528,0.5) ──────
        else if (komut.charAt(0) == 'f' || komut.charAt(0) == 'F') {
            stopAllAndQueue(); // Tekil komut gelirse sırayı iptal et
            String pars     = komut.substring(1);
            int    virguPos = pars.lastIndexOf(',');

            float hz     = 0.0f;
            float dakika = 0.0f;

            if (virguPos >= 0) {
                String strHz  = pars.substring(0, virguPos);
                String strMin = pars.substring(virguPos + 1);
                strHz.replace(',', '.');
                strMin.replace(',', '.');
                hz     = strHz.toFloat();
                dakika = strMin.toFloat();
            } else {
                String strHz = pars;
                strHz.replace(',', '.');
                hz     = strHz.toFloat();
                dakika = 0.0f;
            }

            if (hz > 0.0f) {
                startFreq(hz);
                playStartTime  = millis();
                playDurationMs = (dakika > 0.0f) ? (unsigned long)(dakika * 60000.0f) : 0;

                String resp = "OK:" + String(hz, 2) + "Hz";
                if (dakika > 0.0f) resp += "," + String(dakika, 2) + "dk";
                sendNotify(resp);
            }
        }

        // ── 4. DURDURMA KOMUTU ────────────────────
        else if (komut.equalsIgnoreCase("STOP")) {
            stopAllAndQueue();
            sendNotify("STOPPED");
        }

        // ── 5. DURUM SORGULAMA ────────────────────
        else if (komut.equalsIgnoreCase("STATUS")) {
            if (isPlaying) {
                unsigned long gecenMs = millis() - playStartTime;
                long          kalanSn = (playDurationMs > 0)
                                         ? (long)((playDurationMs - gecenMs) / 1000)
                                         : -1;
                String s = "PLAYING:" + String(currentFreqHz, 2) + "Hz";
                if (kalanSn >= 0) s += ",kalan:" + String(kalanSn) + "sn";
                s += (waveMode == 2) ? ",PULSE" : ",SQUARE";
                sendNotify(s);
            } else {
                sendNotify("IDLE");
            }
        }

        // ── 6. PİL SEVİYESİ SORGULA ──────────────
        else if (komut.equalsIgnoreCase("GET_BAT") || komut.equalsIgnoreCase("BAT")) {
            int batPct = readBatteryPercentage();
            sendNotify("Q_BAT:" + String(batPct));
        }

        // ── 7. CİHAZ İSMİ DEĞİŞTİR ─────────────────
        else if (komut.startsWith("N_")) {
            String yeniIsim = komut.substring(2);
            yeniIsim.trim();
            if (yeniIsim.length() > 0) {
                prefs.begin("ayarlar", false);
                prefs.putString("cihaz_ismi", yeniIsim);
                prefs.end();
                sendNotify("RENAME:" + yeniIsim);
                delay(500);
                ESP.restart();
            }
        }
    }
};

// ════════════════════════════════════════════════════
//  SETUP & LOOP
// ════════════════════════════════════════════════════
void setup() {
    Serial.begin(115200);
    pinMode(HBRIDGE_IN1_PIN, OUTPUT);
    digitalWrite(HBRIDGE_IN1_PIN, LOW);
    gpio_set_pull_mode((gpio_num_t)HBRIDGE_IN1_PIN, GPIO_PULLDOWN_ONLY);

    pinMode(HBRIDGE_IN2_PIN, OUTPUT);
    digitalWrite(HBRIDGE_IN2_PIN, LOW);
    gpio_set_pull_mode((gpio_num_t)HBRIDGE_IN2_PIN, GPIO_PULLDOWN_ONLY);

    // Kalıcı Hafızadan Cihaz İsmini Oku
    prefs.begin("ayarlar", false);
    cihazIsmi = prefs.getString("cihaz_ismi", "Z-Torus_Rife_1");
    prefs.end();

    // BLE Başlat
    BLEDevice::init(cihazIsmi.c_str());
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService* pService = pServer->createService(SERVICE_UUID);

    pWriteChar = pService->createCharacteristic(
        CHAR_WRITE_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pWriteChar->setCallbacks(new MyWriteCallbacks());

    pNotifyChar = pService->createCharacteristic(
        CHAR_NOTIFY_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pNotifyChar->addDescriptor(new BLE2902());

    pService->start();

    // BLE Reklam Yayını
    BLEAdvertising* pAdv = BLEDevice::getAdvertising();
    pAdv->addServiceUUID(SERVICE_UUID);
    pAdv->setScanResponse(true);
    pAdv->setMinPreferred(0x06);
    pAdv->setMaxPreferred(0x12);
    BLEDevice::startAdvertising();

    Serial.printf("[BLE] Z-Torus C3 Standalone v2.2 Yayın Başladı: '%s'\n", cihazIsmi.c_str());
}

void loop() {
    // ── Süre ve Kuyruk Otomasyonu ────────────────────
    if (isPlaying && playDurationMs > 0) {
        if ((millis() - playStartTime) >= playDurationMs) {
            if (isQueueRunning) {
                playNextInQueue(); // Sıradaki frekansa geç
            } else {
                stopFreq();
                sendNotify("FIN");
                playBuzzerFinishChime(); // ESP32 Donanımsal Buzzer Bitiş Uyarısı
            }
        }
    }

    // ── BLE Yeniden Reklam Yönetimi ─────────────────
    if (!deviceConnected && oldDeviceConnected) {
        delay(300);
        pServer->startAdvertising();
        oldDeviceConnected = false;
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = true;
        // İlk bağlandığında pili anında gönder
        sendNotify("Q_BAT:" + String(readBatteryPercentage()));
        lastBatteryNotifyTime = millis();
    }

    // ── Periyodik Pil Gönderimi (Her 10 Saniyede Bir) ──
    if (deviceConnected && (millis() - lastBatteryNotifyTime >= 10000UL)) {
        lastBatteryNotifyTime = millis();
        sendNotify("Q_BAT:" + String(readBatteryPercentage()));
    }

    delay(50);
}
