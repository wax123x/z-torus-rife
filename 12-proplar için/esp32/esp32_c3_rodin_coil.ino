/*
 * =====================================================
 *  Z-Torus ESP32-C3 SuperMini Dual BLE Rodin Bobini (PEMF) Firmware v3.5
 * =====================================================
 *  Pin Bağlantıları:
 *    - GPIO 4  ➔ 15A 400W Çift MOSFET Modülü PWM/TRIG Girişi
 *    - GPIO 1  ➔ Kulaklık / Sinyal Takip Çıkışı
 *    - GPIO 5  ➔ Donanımsal Buzzer Çıkışı
 *    - GPIO 0  ➔ 2S Pil Voltaj Bölücü (4.7k + 530 ohm)
 * =====================================================
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Preferences.h>
#include "esp_timer.h"
#include "driver/gpio.h"

#define MOSFET_PIN      4   // ESP32-C3 SuperMini 15A 400W MOSFET Sinyal Çıkışı (GPIO 4)
#define AUDIO_PIN       1   // ESP32-C3 SuperMini Kulaklık Sinyal Çıkışı (GPIO 1)
#define BUZZER_PIN_A    6   // GPIO 6 (Modül S Pini - 6.6V Diferansiyel)
#define BUZZER_PIN_B    7   // GPIO 7 (Modül - Pini - 6.6V Diferansiyel)
#define BATTERY_ADC_PIN 0   // ESP32-C3 GPIO 0 (2S Pil Voltaj Bölücü ADC Pini)
#define MAX_QUEUE_SIZE 100  // Maksimum frekans sırası kapasitesi

int readBatteryPercentage() {
    long sumADC = 0;
    for (int i = 0; i < 16; i++) {
        sumADC += analogRead(BATTERY_ADC_PIN);
        delayMicroseconds(100);
    }
    float rawADC = sumADC / 16.0f;
    float adcVoltage = (rawADC / 4095.0f) * 3.3f;
    float batteryVoltage = adcVoltage * 9.36f;

    int percentage = (int)(((batteryVoltage - 6.0f) / (8.15f - 6.0f)) * 100.0f);
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
    playBuzzerTone(1318, 130); delay(30); // Mi (E6)
    playBuzzerTone(1568, 130); delay(30); // Sol (G6)
    playBuzzerTone(2093, 300);            // Do (C7 - Tok Başlangıç)
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

#define SERVICE_UUID      "12340000-0000-1000-8000-00805f9b34fb"
#define CHAR_WRITE_UUID   "12340001-0000-1000-8000-00805f9b34fb"
#define CHAR_NOTIFY_UUID  "12340002-0000-1000-8000-00805f9b34fb"

BLEServer*         pServer     = NULL;
BLECharacteristic* pWriteChar  = NULL;
BLECharacteristic* pNotifyChar = NULL;
bool deviceConnected    = false;
bool oldDeviceConnected = false;

struct FreqItem {
    float hz;
    uint32_t durationMs;
    String name;
};

FreqItem freqQueue[MAX_QUEUE_SIZE];
int  queueCount      = 0;
int  currentQueueIdx = 0;
bool isQueueRunning  = false;
bool isLoopEnabled   = false;

bool          isPlaying      = false;
float         currentFreqHz  = 0.0;
unsigned long playStartTime  = 0;
unsigned long playDurationMs = 0;
int           waveMode       = 1;

Preferences prefs;
String      cihazIsmi;

esp_timer_handle_t  squareTimer = NULL;
volatile bool       pinState    = false;
uint64_t            timerOnUs   = 0;
uint64_t            timerOffUs  = 0;

void IRAM_ATTR timerCallback(void* arg) {
    pinState = !pinState;
    gpio_set_level((gpio_num_t)MOSFET_PIN, pinState ? 1 : 0);
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
    ledcDetach(MOSFET_PIN);
    ledcDetach(AUDIO_PIN);
#else
    ledcDetachPin(MOSFET_PIN);
    ledcDetachPin(AUDIO_PIN);
#endif
    gpio_reset_pin((gpio_num_t)MOSFET_PIN);
    gpio_reset_pin((gpio_num_t)AUDIO_PIN);
}

void stopFreq() {
    isPlaying      = false;
    currentFreqHz  = 0.0;
    playDurationMs = 0;
    
    stopSoftwareTimer();
    stopLedc();

    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, LOW);
    gpio_set_pull_mode((gpio_num_t)MOSFET_PIN, GPIO_PULLDOWN_ONLY);

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
void processCommand(String rxValue);

void startFreq(float hz) {
    if (hz <= 0.0f) {
        stopFreq();
        return;
    }

    stopSoftwareTimer();
    stopLedc();

    currentFreqHz = hz;
    isPlaying     = true;

    if (hz < 1000.0f) {
        uint64_t periodUs = (uint64_t)(1000000.0f / hz);

        if (waveMode == 2) {
            timerOnUs  = (uint64_t)(periodUs * 0.20f);
            if (timerOnUs < 10) timerOnUs = 10;
            timerOffUs = periodUs - timerOnUs;
        } else {
            timerOnUs  = periodUs / 2;
            timerOffUs = periodUs - timerOnUs;
        }

        gpio_config_t io_conf = {};
        io_conf.intr_type    = GPIO_INTR_DISABLE;
        io_conf.mode         = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL << MOSFET_PIN) | (1ULL << AUDIO_PIN);
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
        gpio_set_level((gpio_num_t)MOSFET_PIN, 1);
        gpio_set_level((gpio_num_t)AUDIO_PIN, 1);
        esp_timer_start_once(squareTimer, timerOnUs);
    } else {
        gpio_reset_pin((gpio_num_t)MOSFET_PIN);
        gpio_reset_pin((gpio_num_t)AUDIO_PIN);
        pinMode(MOSFET_PIN, OUTPUT);
        pinMode(AUDIO_PIN, OUTPUT);

        int bits = 8;
        uint32_t maxDuty = (1 << bits);
        uint32_t duty    = (waveMode == 2) ? (uint32_t)(maxDuty * 0.20f) : (uint32_t)(maxDuty * 0.50f);
        uint32_t freqInt = (uint32_t)round(hz);

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
        ledcAttach(MOSFET_PIN, freqInt, bits);
        ledcWrite(MOSFET_PIN, duty);
        ledcAttach(AUDIO_PIN, freqInt, bits);
        ledcWrite(AUDIO_PIN, duty);
#else
        ledcSetup(0, freqInt, bits);
        ledcAttachPin(MOSFET_PIN, 0);
        ledcWrite(0, duty);
        ledcAttachPin(AUDIO_PIN, 0);
#endif
    }
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

    String nStr = "PLAYING:" + String(item.hz, 2) + "," + String(item.durationMs / 1000) + "," + String(currentQueueIdx + 1) + "/" + String(queueCount) + "," + item.name;
    sendNotify(nStr);
}

void sendNotify(String msg) {
    if (deviceConnected && pNotifyChar != NULL) {
        pNotifyChar->setValue(msg.c_str());
        pNotifyChar->notify();
    }
}

String getStatusString() {
    String stateStr = isPlaying ? "PLAYING" : "IDLE";
    int pct = readBatteryPercentage();
    String st = "STATUS:" + stateStr + "," + String(currentFreqHz, 2) + "," + String(waveMode) + "," + String(pct) + "%";

    if (isQueueRunning) {
        st += ",QRUN:" + String(currentQueueIdx + 1) + "/" + String(queueCount);
    }
    return st;
}

void processCommand(String rxValue) {
    rxValue.trim();
    if (rxValue.length() == 0) return;

    if (rxValue == "Q_CLR") {
        stopAllAndQueue();
        sendNotify("Q_CLEARED");
        playBuzzerBeep();
    }
    else if (rxValue.startsWith("Q_ADD:")) {
        String payload = rxValue.substring(6);
        int c1 = payload.indexOf(',');
        int c2 = payload.indexOf(',', c1 + 1);

        if (c1 > 0 && queueCount < MAX_QUEUE_SIZE) {
            float hz = payload.substring(0, c1).toFloat();
            float mins = (c2 > 0) ? payload.substring(c1 + 1, c2).toFloat() : payload.substring(c1 + 1).toFloat();
            String name = (c2 > 0) ? payload.substring(c2 + 1) : ("Hz_" + String(hz, 1));

            freqQueue[queueCount].hz         = hz;
            freqQueue[queueCount].durationMs = (uint32_t)(mins * 60.0f * 1000.0f);
            freqQueue[queueCount].name       = name;
            queueCount++;

            sendNotify("Q_ADDED:" + String(queueCount));
        }
    }
    else if (rxValue == "Q_START") {
        if (queueCount > 0) {
            currentQueueIdx = 0;
            isQueueRunning  = true;
            playBuzzerStartTherapyBeep();
            playNextInQueue();
        } else {
            sendNotify("ERR:QUEUE_EMPTY");
        }
    }
    else if (rxValue.startsWith("Q_LOOP:")) {
        isLoopEnabled = (rxValue.substring(7).toInt() == 1);
        sendNotify("Q_LOOP_SET:" + String(isLoopEnabled ? 1 : 0));
        playBuzzerBeep();
    }
    else if (rxValue == "w1") {
        waveMode = 1;
        if (isPlaying) startFreq(currentFreqHz);
        sendNotify("WAVE:SQUARE");
        playBuzzerBeep();
    } else if (rxValue == "w2") {
        waveMode = 2;
        if (isPlaying) startFreq(currentFreqHz);
        sendNotify("WAVE:PULSE");
        playBuzzerBeep();
    }
    else if (rxValue == "STOP") {
        stopAllAndQueue();
        playBuzzerStopBeep();
        sendNotify("STOPPED");
    }
    else if (rxValue == "STATUS") {
        sendNotify(getStatusString());
    }
    else if (rxValue.startsWith("NAME:")) {
        String newName = rxValue.substring(5);
        newName.trim();
        if (newName.length() > 0 && newName.length() <= 20) {
            prefs.begin("ayarlar", false);
            prefs.putString("cihaz_ismi", newName);
            prefs.end();
            sendNotify("NAME_SET:" + newName);
            playBuzzerBeep();
        }
    }
    else if (rxValue.startsWith("f")) {
        isQueueRunning = false;
        queueCount     = 0;
        String payload = rxValue.substring(1);
        int commaIdx   = payload.indexOf(',');

        if (commaIdx > 0) {
            float hz    = payload.substring(0, commaIdx).toFloat();
            float mins  = payload.substring(commaIdx + 1).toFloat();
            playDurationMs = (uint32_t)(mins * 60.0f * 1000.0f);
            playStartTime  = millis();
            playBuzzerStartTherapyBeep();
            startFreq(hz);
            sendNotify("PLAYING:" + String(hz, 2) + "," + String((uint32_t)(mins * 60)));
        } else {
            float hz = payload.toFloat();
            playDurationMs = 0;
            playStartTime  = millis();
            playBuzzerStartTherapyBeep();
            startFreq(hz);
            sendNotify("PLAYING:" + String(hz, 2) + ",0");
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
        String rxValue = pChar->getValue().c_str();
        processCommand(rxValue);
    }
};

void setup() {
    Serial.begin(115200);
    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, LOW);
    gpio_set_pull_mode((gpio_num_t)MOSFET_PIN, GPIO_PULLDOWN_ONLY);

    pinMode(AUDIO_PIN, OUTPUT);
    digitalWrite(AUDIO_PIN, LOW);

    pinMode(BUZZER_PIN_A, OUTPUT);
    digitalWrite(BUZZER_PIN_A, LOW);
    pinMode(BUZZER_PIN_B, OUTPUT);
    digitalWrite(BUZZER_PIN_B, LOW);

    prefs.begin("ayarlar", false);
    cihazIsmi = prefs.getString("cihaz_ismi", "ZTorus_PEMF");
    prefs.end();

    BLEDevice::init(cihazIsmi.c_str());
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
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);
        pServer->startAdvertising();
        oldDeviceConnected = deviceConnected;
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }

    if (millis() - lastBatteryNotifyTime > 30000) {
        lastBatteryNotifyTime = millis();
        sendNotify(getStatusString());
    }

    if (isPlaying && !isQueueRunning && playDurationMs > 0) {
        if (millis() - playStartTime >= playDurationMs) {
            stopFreq();
            sendNotify("FIN");
            playBuzzerFinishChime();
        }
    }

    if (isPlaying && isQueueRunning && playDurationMs > 0) {
        if (millis() - playStartTime >= playDurationMs) {
            currentQueueIdx++;
            playNextInQueue();
        }
    }

    delay(10);
}
