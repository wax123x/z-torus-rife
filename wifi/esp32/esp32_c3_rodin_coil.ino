/*
 * =====================================================
 *  Z-Torus ESP32-C3 SuperMini Dual BLE & WiFi Rodin Bobini (PEMF) Firmware v3.3
 * =====================================================
 *  ÜÇLÜ AKILLI BAĞLANTI (TRIPLE SMART CONNECTION):
 *    1. BLE GATT Server (Her zaman arka planda aktif - Android/PC Web Bluetooth)
 *    2. Ev WiFi Ağı (STA Modu - Kayıtlı ise ilk 6 sn bağlanmayı dener)
 *    3. Kendi WiFi Ağı (SoftAP Modu - Z-TORUS-PEMF / 192.168.4.1)
 *    4. WebSockets Server (Port 81) & HTTP WebServer (Port 80)
 * =====================================================
 *  Pin Bağlantıları:
 *    - GPIO 4  ➔ 15A 400W Çift MOSFET Modülü PWM/TRIG Girişi
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

#define MOSFET_PIN      4
#define AUDIO_PIN       1
#define BUZZER_PIN      5
#define BATTERY_ADC_PIN 0
#define MAX_QUEUE_SIZE 100

WebServer httpServer(80);
WebSocketsServer webSocket(81);

int readBatteryPercentage() {
    long sumADC = 0;
    for (int i = 0; i < 16; i++) {
        sumADC += analogRead(BATTERY_ADC_PIN);
        delayMicroseconds(100);
    }
    float rawADC = sumADC / 16.0f;
    float adcVoltage = (rawADC / 4095.0f) * 3.3f;
    float batteryVoltage = adcVoltage * 8.75f;

    int percentage = (int)(((batteryVoltage - 6.0f) / (8.4f - 6.0f)) * 100.0f);
    if (percentage > 100) percentage = 100;
    if (percentage < 0) percentage = 0;
    return percentage;
}

unsigned long lastBatteryNotifyTime = 0;

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
    playBuzzerTone(1047, 100); delay(50);
    playBuzzerTone(1318, 100); delay(50);
    playBuzzerTone(1568, 150); delay(50);
    playBuzzerTone(2093, 300);
    digitalWrite(BUZZER_PIN, LOW);
}

void playBuzzerStartChime() {
    playBuzzerTone(1047, 80); delay(30);
    playBuzzerTone(1318, 80); delay(30);
    playBuzzerTone(1568, 150);
    digitalWrite(BUZZER_PIN, LOW);
}

void playBuzzerStopBeep() {
    playBuzzerTone(1568, 100); delay(40);
    playBuzzerTone(1047, 200);
    digitalWrite(BUZZER_PIN, LOW);
}

void playBuzzerConnectBeep() {
    playBuzzerTone(1760, 60); delay(30);
    playBuzzerTone(2093, 100);
    digitalWrite(BUZZER_PIN, LOW);
}

void playBuzzerDisconnectBeep() {
    playBuzzerTone(2093, 60); delay(30);
    playBuzzerTone(1318, 100);
    digitalWrite(BUZZER_PIN, LOW);
}

void playBuzzerBeep() {
    playBuzzerTone(2093, 120);
    digitalWrite(BUZZER_PIN, LOW);
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
    webSocket.broadcastTXT(msg.c_str());
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
            playBuzzerBeep();
        }
    }
    else if (rxValue == "Q_START") {
        if (queueCount > 0) {
            currentQueueIdx = 0;
            isQueueRunning  = true;
            playBuzzerStartChime();
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
    else if (rxValue.startsWith("SET_WIFI:")) {
        String payload = rxValue.substring(9);
        int comma = payload.indexOf(',');
        if (comma > 0) {
            String s = payload.substring(0, comma);
            String p = payload.substring(comma + 1);
            s.trim(); p.trim();
            prefs.begin("ayarlar", false);
            prefs.putString("wifi_ssid", s);
            prefs.putString("wifi_pass", p);
            prefs.end();
            sendNotify("WIFI_SAVED:" + s);
            playBuzzerStartChime();
        }
    }
    else if (rxValue == "CLEAR_WIFI") {
        prefs.begin("ayarlar", false);
        prefs.remove("wifi_ssid");
        prefs.remove("wifi_pass");
        prefs.end();
        sendNotify("WIFI_CLEARED");
        playBuzzerStopBeep();
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
            playBuzzerStartChime();
            startFreq(hz);
            sendNotify("PLAYING:" + String(hz, 2) + "," + String((uint32_t)(mins * 60)));
        } else {
            float hz = payload.toFloat();
            playDurationMs = 0;
            playStartTime  = millis();
            playBuzzerStartChime();
            startFreq(hz);
            sendNotify("PLAYING:" + String(hz, 2) + ",0");
        }
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            break;
        case WStype_CONNECTED: {
            webSocket.sendTXT(num, getStatusString().c_str());
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

const char INDEX_HTML[] PROGMEM = "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>Z-TORUS</title><style>body{background:#0f172a;color:#e2e8f0;font-family:sans-serif;text-align:center;padding:30px 15px}.card{background:#1e293b;padding:25px;border-radius:16px;max-width:400px;margin:auto;border:1px solid #334155}h1{color:#10b981;font-size:22px}p{color:#94a3b8;font-size:14px}.status{background:#065f46;color:#34d399;padding:8px 16px;border-radius:20px;display:inline-block;font-weight:bold;margin:15px 0}.btn{background:#3b82f6;color:#fff;padding:12px 24px;border-radius:10px;text-decoration:none;font-weight:bold;display:block;margin:10px 0}</style></head><body><div class='card'><h1>⚡ Z-TORUS PEMF</h1><div class='status'>🟢 Cihaz Aktif</div><p>ESP32-C3 WiFi & WebSockets Hazır.</p><a href='/status' class='btn'>📊 Durum İste</a></div></body></html>";

void setupWiFi() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Z-TORUS-PEMF");

    httpServer.on("/", []() {
        httpServer.send_P(200, "text/html", INDEX_HTML);
    });

    httpServer.on("/status", []() {
        httpServer.send(200, "text/plain", getStatusString());
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

void setup() {
    Serial.begin(115200);
    pinMode(MOSFET_PIN, OUTPUT);
    digitalWrite(MOSFET_PIN, LOW);
    gpio_set_pull_mode((gpio_num_t)MOSFET_PIN, GPIO_PULLDOWN_ONLY);

    pinMode(AUDIO_PIN, OUTPUT);
    digitalWrite(AUDIO_PIN, LOW);

    prefs.begin("ayarlar", false);
    cihazIsmi = prefs.getString("cihaz_ismi", "ZTorus_PEMF");
    prefs.end();

    setupWiFi();

    BLEDevice::init(cihazIsmi.c_str());
    BLEDevice::setPower(ESP_PWR_LVL_P21);

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
