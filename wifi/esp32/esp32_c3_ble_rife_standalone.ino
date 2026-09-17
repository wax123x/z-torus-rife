/*
 * =====================================================
 *  Z-Torus ESP32-C3 SuperMini Dual BLE Standalone Rife (El Propları / Elektrot) Firmware v3.5
 * =====================================================
 *  Pin Bağlantıları (DRV8871 Çift Fazlı Biphasic AC H-Köprüsü):
 *    - GPIO 4  ➔ DRV8871 IN1 Çıkışı (Faz A)
 *    - GPIO 3  ➔ DRV8871 IN2 Çıkışı (Faz B - Zıt Faz Biphasic AC)
 *    - GPIO 1  ➔ Kulaklık / Sinyal Takip Çıkışı
 *    - GPIO 5  ➔ Donanımsal Buzzer Çıkışı
 *    - GPIO 0  ➔ 2S Pil Voltaj Bölücü (4.7k + 530 ohm)
 * =====================================================
 */

#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>
#include "esp_timer.h"
#include "driver/gpio.h"

#define HBRIDGE_IN1_PIN 4
#define HBRIDGE_IN2_PIN 3
#define AUDIO_PIN       1
#define BUZZER_PIN_A    6   // GPIO 6 (Modül S Pini - 6.6V Diferansiyel)
#define BUZZER_PIN_B    7   // GPIO 7 (Modül - Pini - 6.6V Diferansiyel)
#define BATTERY_ADC_PIN 0
#define STATUS_LED_PIN  10  // 3mm Durum Gösterge LED Pini (GPIO 10)
#define MAX_QUEUE_SIZE 100

WebServer httpServer(80);
WebSocketsServer webSocket = WebSocketsServer(81);

int readBatteryPercentage() {
    long sumADC = 0;
    for (int i = 0; i < 16; i++) {
        sumADC += analogRead(BATTERY_ADC_PIN);
        delayMicroseconds(100);
    }
    float rawADC = sumADC / 16.0f;
    float adcVoltage = (rawADC / 4095.0f) * 3.3f;
    float batteryVoltage = adcVoltage * 8.91f;

    int percentage = (int)(((batteryVoltage - 6.0f) / (8.4f - 6.0f)) * 100.0f);
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    return percentage;
}

unsigned long lastBatteryNotifyTime = 0;

// ── 6.6V Diferansiyel Köprü Buzzer Sürücüsü (Zıt Fazlı Maksimum Güç) ──
void playBuzzerTone(uint32_t freq, uint32_t durMs) {
    if (freq == 0) {
        digitalWrite(BUZZER_PIN_A, LOW);
        digitalWrite(BUZZER_PIN_B, LOW);
        delay(durMs);
        return;
    }
    gpio_set_drive_capability((gpio_num_t)BUZZER_PIN_A, GPIO_DRIVE_CAP_3); // Maksimum 40mA Akım Gücü
    gpio_set_drive_capability((gpio_num_t)BUZZER_PIN_B, GPIO_DRIVE_CAP_3); // Maksimum 40mA Akım Gücü

    pinMode(BUZZER_PIN_A, OUTPUT);
    pinMode(BUZZER_PIN_B, OUTPUT);

    unsigned long startMs = millis();
    unsigned long halfPeriodUs = 1000000UL / (freq * 2);
    bool phase = false;

    while (millis() - startMs < durMs) {
        phase = !phase;
        digitalWrite(BUZZER_PIN_A, phase ? HIGH : LOW);
        digitalWrite(BUZZER_PIN_B, phase ? LOW : HIGH);
        delayMicroseconds(halfPeriodUs);
    }

    digitalWrite(BUZZER_PIN_A, LOW);
    digitalWrite(BUZZER_PIN_B, LOW);
}

void playBuzzerFinishChime() {
    // ⚔️ Tedavi Bitti Fanfarı (dta-da-da-daaaam! Varyasyon 2)
    playBuzzerTone(1568, 90); delay(25);  // dta
    playBuzzerTone(1568, 90); delay(25);  // da
    playBuzzerTone(1568, 90); delay(25);  // da
    playBuzzerTone(2093, 800);           // DAAAAM! (Dev Do Çanı)
    digitalWrite(BUZZER_PIN_A, LOW);
    digitalWrite(BUZZER_PIN_B, LOW);
}

void playBuzzerStartChime() {
    // 🪟 Windows Açılış Melodisi (Efsanevi Başlama Çanı)
    playBuzzerTone(1244, 160); delay(40); // Eb6
    playBuzzerTone(1661, 160); delay(40); // Ab6
    playBuzzerTone(1864, 180); delay(40); // Bb6
    playBuzzerTone(2489, 550);            // Eb7 (Uzun Süzülen Windows Çanı)
    digitalWrite(BUZZER_PIN_A, LOW);
    digitalWrite(BUZZER_PIN_B, LOW);
}

void playBuzzerStartTherapyBeep() {
    playBuzzerTone(1568, 140); delay(80); // Sol (1568 Hz - Gür Rezonans, 80ms Net Es)
    playBuzzerTone(1760, 140); delay(80); // La  (1760 Hz - Yükseliş, 80ms Net Es)
    playBuzzerTone(2093, 400);            // Do  (2093 Hz - Görkemli Net Çan)
    digitalWrite(BUZZER_PIN_A, LOW);
    digitalWrite(BUZZER_PIN_B, LOW);
}

void playBuzzerStopBeep() {
    playBuzzerTone(1568, 160); delay(40); // Sol (G6)
    playBuzzerTone(1318, 160); delay(40); // Mi (E6)
    playBuzzerTone(1047, 300);            // Do (C6 - Tok Durma)
    digitalWrite(BUZZER_PIN_A, LOW);
    digitalWrite(BUZZER_PIN_B, LOW);
}

void playBuzzerConnectBeep() {
    playBuzzerTone(1568, 140); delay(40); // Sol (G6)
    playBuzzerTone(2093, 250);            // Do (C7 - Tok Onay)
    digitalWrite(BUZZER_PIN_A, LOW);
    digitalWrite(BUZZER_PIN_B, LOW);
}

void playBuzzerDisconnectBeep() {
    playBuzzerTone(2093, 140); delay(40); // Do (C7)
    playBuzzerTone(1318, 250);            // Mi (E6 - Tok Uyarı)
    digitalWrite(BUZZER_PIN_A, LOW);
    digitalWrite(BUZZER_PIN_B, LOW);
}

void playBuzzerBeep() {
    playBuzzerTone(2093, 150);
    digitalWrite(BUZZER_PIN_A, LOW);
    digitalWrite(BUZZER_PIN_B, LOW);
}

#define SERVICE_UUID     "12340000-0000-1000-8000-00805f9b34fb"
#define CHAR_WRITE_UUID  "12340001-0000-1000-8000-00805f9b34fb"
#define CHAR_NOTIFY_UUID "12340002-0000-1000-8000-00805f9b34fb"

BLEServer*         pServer            = NULL;
BLECharacteristic* pWriteChar         = NULL;
BLECharacteristic* pNotifyChar        = NULL;

bool               deviceConnected    = false;
bool               oldDeviceConnected = false;

volatile bool      isPlaying          = false;
volatile float     currentFreqHz      = 0.0;
unsigned long      playStartTime      = 0;
unsigned long      playDurationMs     = 0;
int                waveMode           = 1;

struct FreqItem {
    float         hz;
    unsigned long durationMs;
};

FreqItem           freqQueue[MAX_QUEUE_SIZE];
int                queueCount         = 0;
int                currentQueueIdx    = 0;
bool               isQueueRunning     = false;
bool               isLoopEnabled      = false;

Preferences prefs;
String      cihazIsmi;

esp_timer_handle_t  squareTimer = NULL;
volatile bool       pinState    = false;
uint64_t            timerOnUs   = 0;
uint64_t            timerOffUs  = 0;

void IRAM_ATTR timerCallback(void* arg) {
    pinState = !pinState;
    gpio_set_level((gpio_num_t)HBRIDGE_IN1_PIN, pinState ? 1 : 0);
    gpio_set_level((gpio_num_t)HBRIDGE_IN2_PIN, pinState ? 0 : 1);
    gpio_set_level((gpio_num_t)AUDIO_PIN, pinState ? 1 : 0);

    if (squareTimer != NULL && isPlaying) {
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
}

void stopAllAndQueue() {
    isQueueRunning = false;
    queueCount     = 0;
    currentQueueIdx= 0;
    stopFreq();
}

void sendNotify(String msg);
void processCommand(String komut);

void startFreq(float hz) {
    if (hz <= 0.0f) {
        stopFreq();
        return;
    }

    stopSoftwareTimer();
    stopLedc();

    currentFreqHz = hz;
    isPlaying     = true;

    uint64_t periodUs = (uint64_t)(1000000.0f / hz);

    if (waveMode == 2) {
        timerOnUs  = (uint64_t)(periodUs * 0.20f);
        if (timerOnUs < 5) timerOnUs = 5;
        timerOffUs = periodUs - timerOnUs;
    } else {
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
    gpio_set_level((gpio_num_t)HBRIDGE_IN2_PIN, 0);
    gpio_set_level((gpio_num_t)AUDIO_PIN, 1);
    esp_timer_start_once(squareTimer, timerOnUs);
}

void playNextInQueue() {
    if (queueCount == 0 || currentQueueIdx >= queueCount) {
        if (isLoopEnabled && queueCount > 0) {
            currentQueueIdx = 0;
        } else {
            stopAllAndQueue();
            sendNotify("FIN");
            playBuzzerFinishChime();
            return;
        }
    }

    FreqItem item = freqQueue[currentQueueIdx];
    startFreq(item.hz);
    playStartTime  = millis();
    playDurationMs = item.durationMs;

    String notifyStr = "Q_NEXT:" + String(currentQueueIdx) + ":" + String(item.hz, 2);
    sendNotify(notifyStr);
    currentQueueIdx++;
}

void sendNotify(String msg) {
    if (deviceConnected && pNotifyChar) {
        pNotifyChar->setValue(msg.c_str());
        pNotifyChar->notify();
    }
    webSocket.broadcastTXT(msg.c_str());
}

void processCommand(String komut) {
    komut.trim();
    if (komut.length() == 0) return;

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
                freqQueue[queueCount].durationMs = (min > 0.0f) ? ((unsigned long)(min * 60000.0f)) : 300000UL;
                queueCount++;
                sendNotify("Q_ADD_OK:" + String(queueCount));
            }
        }
    }
    else if (komut.equalsIgnoreCase("Q_START")) {
        if (queueCount > 0) {
            currentQueueIdx = 0;
            isQueueRunning  = true;
            playBuzzerStartTherapyBeep();
            playNextInQueue();
        }
    }
    else if (komut.equalsIgnoreCase("w0") || komut.equalsIgnoreCase("w1")) {
        waveMode = 1;
        sendNotify("MODE:SQUARE");
        if (isPlaying && currentFreqHz > 0) startFreq(currentFreqHz);
    }
    else if (komut.equalsIgnoreCase("w2")) {
        waveMode = 2;
        sendNotify("MODE:PULSE");
        if (isPlaying && currentFreqHz > 0) startFreq(currentFreqHz);
    }
    else if (komut.charAt(0) == 'f' || komut.charAt(0) == 'F') {
        stopAllAndQueue();
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
            playBuzzerStartTherapyBeep();
            startFreq(hz);
            playStartTime  = millis();
            playDurationMs = (dakika > 0.0f) ? (unsigned long)(dakika * 60000.0f) : 0;

            String resp = "OK:" + String(hz, 2) + "Hz";
            if (dakika > 0.0f) resp += "," + String(dakika, 2) + "dk";
            sendNotify(resp);
        }
    }
    else if (komut.equalsIgnoreCase("STOP")) {
        stopAllAndQueue();
        playBuzzerStopBeep();
        sendNotify("STOPPED");
    }
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
    else if (komut.equalsIgnoreCase("GET_BAT") || komut.equalsIgnoreCase("BAT")) {
        int batPct = readBatteryPercentage();
        sendNotify("Q_BAT:" + String(batPct));
    }
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

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
        playBuzzerConnectBeep();
    }
    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
        playBuzzerDisconnectBeep();
    }
};

class MyWriteCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pChar) override {
        String komut = pChar->getValue();
        processCommand(komut);
    }
};

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            break;
        case WStype_CONNECTED: {
            webSocket.sendTXT(num, isPlaying ? "PLAYING" : "IDLE");
            break;
        }
        case WStype_TEXT: {
            String cmd = String((char*)payload);
            cmd.trim();
            if (cmd.length() > 0) {
                processCommand(cmd);
            }
            break;
        }
        default:
            break;
    }
}

const char INDEX_HTML[] PROGMEM = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Z-TORUS RIFE</title><style>body{background:#0f172a;color:#e2e8f0;font-family:sans-serif;text-align:center;padding:30px 15px}.card{background:#1e293b;padding:25px;border-radius:16px;max-width:400px;margin:auto;border:1px solid #334155}h1{color:#3b82f6;font-size:22px}p{color:#94a3b8;font-size:14px}.status{background:#1e3a8a;color:#60a5fa;padding:8px 16px;border-radius:20px;display:inline-block;font-weight:bold;margin:15px 0}.btn{background:#10b981;color:#fff;padding:12px 24px;border-radius:10px;text-decoration:none;font-weight:bold;display:block;margin:10px 0}</style></head><body><div class='card'><h1>⚡ Z-TORUS RIFE</h1><div class='status'>🟢 Cihaz Aktif</div><p>ESP32-C3 WiFi & WebSockets Hazır.</p><a href='/status' class='btn'>📊 Durum İste</a></div></body></html>";

void setupWiFi() {
    WiFi.mode(WIFI_AP);
    WiFi.setTxPower(WIFI_POWER_19_5dBm); // ESP32-C3 Maksimum WiFi Çıkış Gücü (+19.5 dBm)
    WiFi.softAP("Z-TORUS-RIFE");

    httpServer.on("/", []() {
        httpServer.send_P(200, "text/html", INDEX_HTML);
    });

    httpServer.on("/status", []() {
        httpServer.send(200, "text/plain", isPlaying ? "PLAYING" : "IDLE");
    });

    httpServer.on("/cmd", []() {
        if (httpServer.hasArg("c")) {
            String c = httpServer.arg("c");
            processCommand(c);
            httpServer.send(200, "text/plain", "OK");
        } else {
            httpServer.send(400, "text/plain", "ERR");
        }
    });

    httpServer.onNotFound([]() {
        httpServer.sendHeader("Location", "http://192.168.4.1/", true);
        httpServer.send(302, "text/plain", "");
    });

    httpServer.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void updateStatusLED() {
    static unsigned long lastLedUpdate = 0;
    static unsigned long lastBatCheckTime = 0;
    static int cachedBatPct = 100;
    unsigned long now = millis();

    // Pil ölçümünü 2 saniyede bir güncelle (AnalogRead işlem yükünü optimize etmek için)
    if (now - lastBatCheckTime >= 2000 || lastBatCheckTime == 0) {
        lastBatCheckTime = now;
        cachedBatPct = readBatteryPercentage();
    }

    // ── DURUM 1: Düşük Pil Uyarısı (<= %25) ➔ 4 Hz Hızlı Flaşör (125ms AÇIK / 125ms KAPALI) ──
    if (cachedBatPct <= 25) {
        if (now - lastLedUpdate >= 125) {
            lastLedUpdate = now;
            digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
        }
        return;
    }

    // ── DURUM 2: Sinyal Terapi Aktif (isPlaying == true) ➔ Çift Atışlı Kalp Atışı ("lub-dub") ──
    if (isPlaying) {
        unsigned long cycleMs = now % 1000; // 1 saniyelik tekrarlayan döngü
        // 0-80ms: Lub (AÇIK) | 80-200ms: Es | 200-280ms: Dub (AÇIK) | 280-1000ms: Dinlenme
        if ((cycleMs >= 0 && cycleMs < 80) || (cycleMs >= 200 && cycleMs < 280)) {
            digitalWrite(STATUS_LED_PIN, HIGH);
        } else {
            digitalWrite(STATUS_LED_PIN, LOW);
        }
        return;
    }

    // ── DURUM 3: Sistem Açık / Bekleme Modu ➔ Sürekli Düz Yanık ──
    digitalWrite(STATUS_LED_PIN, HIGH);
}

void setup() {
    Serial.begin(115200);
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, HIGH); // Güç ilk verildiğinde düz yanacak

    pinMode(HBRIDGE_IN1_PIN, OUTPUT);
    digitalWrite(HBRIDGE_IN1_PIN, LOW);
    gpio_set_pull_mode((gpio_num_t)HBRIDGE_IN1_PIN, GPIO_PULLDOWN_ONLY);

    pinMode(HBRIDGE_IN2_PIN, OUTPUT);
    digitalWrite(HBRIDGE_IN2_PIN, LOW);
    gpio_set_pull_mode((gpio_num_t)HBRIDGE_IN2_PIN, GPIO_PULLDOWN_ONLY);

    pinMode(BUZZER_PIN_A, OUTPUT);
    digitalWrite(BUZZER_PIN_A, LOW);
    pinMode(BUZZER_PIN_B, OUTPUT);
    digitalWrite(BUZZER_PIN_B, LOW);

    prefs.begin("ayarlar", false);
    cihazIsmi = prefs.getString("cihaz_ismi", "ZTorus_Rife");
    prefs.end();

    setupWiFi();

    BLEDevice::init(cihazIsmi.c_str());
    // ESP32-C3 BLE Verici Gücünü Tüm Modlarda Maksimum +21 dBm'e (ESP_PWR_LVL_P21) Kilitle
    BLEDevice::setPower(ESP_PWR_LVL_P21, ESP_BLE_PWR_TYPE_DEFAULT);
    BLEDevice::setPower(ESP_PWR_LVL_P21, ESP_BLE_PWR_TYPE_ADV);
    BLEDevice::setPower(ESP_PWR_LVL_P21, ESP_BLE_PWR_TYPE_CONN_HDL0);
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

    BLEAdvertising* pAdv = BLEDevice::getAdvertising();
    pAdv->addServiceUUID(SERVICE_UUID);
    pAdv->setScanResponse(true);
    pAdv->setMinPreferred(0x06);
    pAdv->setMaxPreferred(0x12);
    BLEDevice::startAdvertising();

    playBuzzerStartChime();
}

void loop() {
    httpServer.handleClient();
    webSocket.loop();
    updateStatusLED();

    if (isPlaying && playDurationMs > 0) {
        if ((millis() - playStartTime) >= playDurationMs) {
            if (isQueueRunning) {
                playNextInQueue();
            } else {
                stopFreq();
                sendNotify("FIN");
                playBuzzerFinishChime();
            }
        }
    }

    if (!deviceConnected && oldDeviceConnected) {
        delay(300);
        pServer->startAdvertising();
        oldDeviceConnected = false;
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = true;
        sendNotify("Q_BAT:" + String(readBatteryPercentage()));
        lastBatteryNotifyTime = millis();
    }

    if (deviceConnected && (millis() - lastBatteryNotifyTime >= 10000UL)) {
        lastBatteryNotifyTime = millis();
        sendNotify("Q_BAT:" + String(readBatteryPercentage()));
    }

    delay(10);
}
