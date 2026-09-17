/*
 * =====================================================
 *  Z-Torus Rodin Bobini Endüktans (L) Ölçüm Aracı
 * =====================================================
 *  AMAÇ: Bobine paralel bağlanan bilinen kapasitör ile
 *         rezonans frekansını bularak L değerini hesaplar.
 *
 *  KULLANIM:
 *    1. Bobine paralel bir kapasitör bağlayın (ör. 10µF)
 *    2. Bu firmware'i yükleyin
 *    3. Serial Monitor'ü 115200 baud ile açın
 *    4. 's' tuşuna basarak taramayı başlatın
 *    5. En güçlü titreşimi/uğultuyu hissettiğiniz frekansı not alın
 *    6. Firmware otomatik olarak L değerini hesaplar
 *
 *  Pin Bağlantıları (mevcut sisteminizle aynı):
 *    - GPIO 4  → MOSFET 1 (Bobin + Kapasitör bağlı)
 *    - GPIO 6  → Buzzer A (Diferansiyel)
 *    - GPIO 7  → Buzzer B (Diferansiyel)
 * =====================================================
 */

#include "esp_timer.h"
#include "driver/gpio.h"

#define MOSFET_PIN     4   // AOD4184 MOSFET sürüş pini
#define BUZZER_PIN_A   6   // Buzzer +
#define BUZZER_PIN_B   7   // Buzzer -

// ═══════════════════════════════════════════════
// KAPASİTÖR DEĞERİNİZİ BURAYA GİRİN (Farad cinsinden)
// ═══════════════════════════════════════════════
// 10µF  = 10e-6  = 0.000010
// 22µF  = 22e-6  = 0.000022
// 47µF  = 47e-6  = 0.000047
// 100µF = 100e-6 = 0.000100
// ───────────────────────────────────────────────
float CAPACITOR_FARADS = 10e-6;  // ← Kapasitör değerinizi buraya yazın!
// ═══════════════════════════════════════════════

// Tarama parametreleri
float scanStartHz   = 10.0;     // Tarama başlangıç frekansı
float scanEndHz     = 5000.0;   // Tarama bitiş frekansı
int   scanSteps     = 100;      // Tarama adım sayısı
float stepDuration  = 2.0;      // Her frekansta bekleme süresi (saniye)

// Durum değişkenleri
bool isScanning     = false;
int  currentStep    = 0;
float currentHz     = 0;
float peakHz        = 0;        // Kullanıcının bildirdiği rezonans frekansı

// Timer
esp_timer_handle_t squareTimer = NULL;
volatile bool pinState = false;
uint64_t timerHalfPeriodUs = 0;

// ── Buzzer Fonksiyonları ──
void playBuzzerTone(uint32_t freq, uint32_t durMs) {
    if (freq == 0) { delay(durMs); return; }
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

void playBeep() {
    playBuzzerTone(2093, 100);
}

void playStartBeep() {
    playBuzzerTone(1568, 120); delay(30);
    playBuzzerTone(2093, 200);
}

void playEndBeep() {
    playBuzzerTone(2093, 100); delay(30);
    playBuzzerTone(1568, 100); delay(30);
    playBuzzerTone(2093, 300);
}

// ── MOSFET Kare Dalga Üretici ──
void IRAM_ATTR timerCallback(void* arg) {
    pinState = !pinState;
    gpio_set_level((gpio_num_t)MOSFET_PIN, pinState ? 1 : 0);

    if (squareTimer != NULL && isScanning) {
        esp_timer_start_once(squareTimer, timerHalfPeriodUs);
    }
}

void stopTimer() {
    if (squareTimer != NULL) {
        esp_timer_stop(squareTimer);
        esp_timer_delete(squareTimer);
        squareTimer = NULL;
    }
    gpio_set_level((gpio_num_t)MOSFET_PIN, 0);
    pinState = false;
}

void startFreq(float hz) {
    stopTimer();
    
    if (hz <= 0) return;
    
    currentHz = hz;
    uint64_t periodUs = (uint64_t)(1000000.0f / hz);
    timerHalfPeriodUs = periodUs / 2;
    
    if (timerHalfPeriodUs < 5) timerHalfPeriodUs = 5;
    
    esp_timer_create_args_t timerArgs = {};
    timerArgs.callback        = timerCallback;
    timerArgs.arg             = NULL;
    timerArgs.dispatch_method = ESP_TIMER_TASK;
    timerArgs.name            = "l_test";
    
    esp_timer_create(&timerArgs, &squareTimer);
    
    pinState = true;
    gpio_set_level((gpio_num_t)MOSFET_PIN, 1);
    esp_timer_start_once(squareTimer, timerHalfPeriodUs);
}

// ── Endüktans Hesaplama ──
float calculateInductance(float resonanceHz, float capacitanceFarads) {
    // L = 1 / (4 × π² × f₀² × C)
    float pi2 = 6.283185307f;
    float denominator = pi2 * pi2 * resonanceHz * resonanceHz * capacitanceFarads;
    if (denominator <= 0) return 0;
    return 1.0f / denominator;
}

void printInductanceResult(float resonanceHz) {
    float L = calculateInductance(resonanceHz, CAPACITOR_FARADS);
    
    Serial.println();
    Serial.println("╔══════════════════════════════════════════════╗");
    Serial.println("║     📐 ENDÜKTANS ÖLÇÜM SONUCU              ║");
    Serial.println("╠══════════════════════════════════════════════╣");
    Serial.print("║  Rezonans Frekansı : ");
    Serial.print(resonanceHz, 1);
    Serial.println(" Hz");
    Serial.print("║  Kapasitör Değeri  : ");
    Serial.print(CAPACITOR_FARADS * 1e6, 1);
    Serial.println(" µF");
    Serial.print("║  Hesaplanan L      : ");
    
    if (L >= 1.0) {
        Serial.print(L, 3);
        Serial.println(" H");
    } else if (L >= 0.001) {
        Serial.print(L * 1000.0f, 2);
        Serial.println(" mH");
    } else {
        Serial.print(L * 1000000.0f, 1);
        Serial.println(" µH");
    }
    
    Serial.println("╠══════════════════════════════════════════════╣");
    
    // LC Rezonans için ideal kapasitör önerileri (PEMF frekansları)
    Serial.println("║                                              ║");
    Serial.println("║  📋 PEMF Frekansları İçin Rezonans Kapasitörleri:");
    
    float targetFreqs[] = {7.83, 10.0, 40.0, 100.0, 528.0, 1000.0};
    const char* freqNames[] = {"Schumann", "Alfa", "Gamma", "PEMF-100", "Solfeggio", "PEMF-1k"};
    
    for (int i = 0; i < 6; i++) {
        float Creq = 1.0f / (4.0f * 3.14159f * 3.14159f * targetFreqs[i] * targetFreqs[i] * L);
        Serial.print("║  ");
        Serial.print(targetFreqs[i], 2);
        Serial.print(" Hz (");
        Serial.print(freqNames[i]);
        Serial.print(") → C = ");
        
        if (Creq >= 0.001) {
            Serial.print(Creq * 1000.0f, 1);
            Serial.println(" mF");
        } else if (Creq >= 0.000001) {
            Serial.print(Creq * 1000000.0f, 1);
            Serial.println(" µF");
        } else {
            Serial.print(Creq * 1000000000.0f, 1);
            Serial.println(" nF");
        }
    }
    
    Serial.println("║                                              ║");
    Serial.println("╚══════════════════════════════════════════════╝");
    Serial.println();
    
    // Q faktörü tahmini
    float XL = 2.0f * 3.14159f * resonanceHz * L;
    float Q = XL / 8.0f; // R = 8 ohm
    Serial.print("Tahmini Q Faktörü (R=8Ω): ");
    Serial.println(Q, 1);
    Serial.print("Rezonansta tepe akım tahmini: ");
    Serial.print(3.0f * Q, 1); // 24V/8Ω × Q
    Serial.println(" A (24V besleme ile)");
    Serial.println();
}

// ── Logaritmik Frekans Hesaplama ──
float getLogFreq(int step, int totalSteps, float startHz, float endHz) {
    // Logaritmik tarama (düşük frekanslarda daha hassas)
    float logStart = log10(startHz);
    float logEnd   = log10(endHz);
    float logStep  = logStart + (logEnd - logStart) * ((float)step / (float)totalSteps);
    return pow(10.0f, logStep);
}

// ── Ana Tarama Fonksiyonu ──
void runScan() {
    Serial.println();
    Serial.println("════════════════════════════════════════════");
    Serial.println(" 🔍 FREKANS TARAMASI BAŞLIYOR");
    Serial.println("════════════════════════════════════════════");
    Serial.print(" Aralık: ");
    Serial.print(scanStartHz, 0);
    Serial.print(" Hz → ");
    Serial.print(scanEndHz, 0);
    Serial.println(" Hz");
    Serial.print(" Adım sayısı: ");
    Serial.println(scanSteps);
    Serial.print(" Her adımda bekleme: ");
    Serial.print(stepDuration, 1);
    Serial.println(" sn");
    Serial.print(" Kapasitör: ");
    Serial.print(CAPACITOR_FARADS * 1e6, 1);
    Serial.println(" µF");
    Serial.println("────────────────────────────────────────────");
    Serial.println(" ⚡ Bobine dokunarak veya yakınında");
    Serial.println("    EN GÜÇLÜ titreşimi hissedin!");
    Serial.println("    Ayrıca bobinden gelen uğultuyu dinleyin.");
    Serial.println("────────────────────────────────────────────");
    Serial.println();
    Serial.println(" Adım | Frekans     | Durum");
    Serial.println(" ─────┼─────────────┼──────────────────");
    
    isScanning = true;
    playStartBeep();
    delay(500);
    
    for (int i = 0; i <= scanSteps && isScanning; i++) {
        float hz = getLogFreq(i, scanSteps, scanStartHz, scanEndHz);
        currentStep = i;
        
        startFreq(hz);
        
        // Ekrana yaz
        Serial.print(" ");
        if (i < 10) Serial.print(" ");
        if (i < 100) Serial.print(" ");
        Serial.print(i);
        Serial.print("  | ");
        Serial.print(hz, 1);
        if (hz < 10) Serial.print("    ");
        else if (hz < 100) Serial.print("   ");
        else if (hz < 1000) Serial.print("  ");
        else Serial.print(" ");
        Serial.print(" Hz | ▓");
        
        // Basit görsel bar (frekansa göre)
        int barLen = (int)(hz / scanEndHz * 20);
        for (int b = 0; b < barLen; b++) Serial.print("▓");
        for (int b = barLen; b < 20; b++) Serial.print("░");
        Serial.println("│");
        
        // Bekleme (kullanıcı hissetsin)
        unsigned long waitStart = millis();
        while (millis() - waitStart < (unsigned long)(stepDuration * 1000)) {
            // Serial'den komut kontrolü
            if (Serial.available()) {
                char c = Serial.read();
                if (c == 'x' || c == 'X') {
                    // Taramayı durdur
                    isScanning = false;
                    stopTimer();
                    Serial.println("\n ⛔ Tarama iptal edildi!");
                    return;
                }
                if (c == 'p' || c == 'P') {
                    // Bu frekansı rezonans olarak işaretle
                    peakHz = hz;
                    Serial.println("\n ✅ Bu frekans rezonans olarak işaretlendi!");
                    playBeep();
                }
            }
            delay(10);
        }
    }
    
    stopTimer();
    isScanning = false;
    playEndBeep();
    
    Serial.println();
    Serial.println("════════════════════════════════════════════");
    Serial.println(" ✅ TARAMA TAMAMLANDI!");
    Serial.println("════════════════════════════════════════════");
    
    if (peakHz > 0) {
        printInductanceResult(peakHz);
    } else {
        Serial.println();
        Serial.println(" Rezonans frekansını manuel girmek için:");
        Serial.println(" Serial'e 'r123.4' yazın (123.4 Hz örnek)");
        Serial.println();
    }
}

// ── Tek Frekans Test ──
void testSingleFreq(float hz) {
    Serial.print("▶ Tek frekans çalışıyor: ");
    Serial.print(hz, 1);
    Serial.println(" Hz — Durdurmak için 'x' gönderin");
    
    isScanning = true;
    startFreq(hz);
    
    while (isScanning) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == 'x' || c == 'X') {
                isScanning = false;
            }
        }
        delay(10);
    }
    
    stopTimer();
    Serial.println("⏹ Durduruldu.");
}

// ── Yardım Menüsü ──
void printHelp() {
    Serial.println();
    Serial.println("╔══════════════════════════════════════════════════╗");
    Serial.println("║  Z-Torus Endüktans Ölçüm Aracı v1.0            ║");
    Serial.println("╠══════════════════════════════════════════════════╣");
    Serial.println("║                                                  ║");
    Serial.println("║  KOMUTLAR:                                       ║");
    Serial.println("║  s        → Frekans taraması başlat              ║");
    Serial.println("║  x        → Taramayı / çalmayı durdur            ║");
    Serial.println("║  p        → Mevcut frekansı rezonans işaretle    ║");
    Serial.println("║  f100     → 100 Hz tek frekans test              ║");
    Serial.println("║  f528     → 528 Hz tek frekans test              ║");
    Serial.println("║  r234.5   → 234.5 Hz rezonans olarak gir,       ║");
    Serial.println("║              L hesapla                           ║");
    Serial.println("║  c22      → Kapasitör değerini 22µF olarak ayarla║");
    Serial.println("║  h        → Bu yardım menüsü                    ║");
    Serial.println("║                                                  ║");
    Serial.println("║  TARAMA SIRASINDA:                               ║");
    Serial.println("║  - Bobine elinizi yaklaştırın                    ║");
    Serial.println("║  - En güçlü titreşim = REZONANS                 ║");
    Serial.println("║  - 'p' ile o frekansı işaretleyin                ║");
    Serial.println("║                                                  ║");
    Serial.println("╚══════════════════════════════════════════════════╝");
    Serial.println();
    Serial.print("Kapasitör ayarı: ");
    Serial.print(CAPACITOR_FARADS * 1e6, 1);
    Serial.println(" µF");
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    // MOSFET pin
    gpio_reset_pin((gpio_num_t)MOSFET_PIN);
    gpio_set_direction((gpio_num_t)MOSFET_PIN, GPIO_MODE_OUTPUT);
    gpio_set_drive_capability((gpio_num_t)MOSFET_PIN, GPIO_DRIVE_CAP_3);
    gpio_set_level((gpio_num_t)MOSFET_PIN, 0);
    
    // Buzzer pinleri
    pinMode(BUZZER_PIN_A, OUTPUT);
    digitalWrite(BUZZER_PIN_A, LOW);
    pinMode(BUZZER_PIN_B, OUTPUT);
    digitalWrite(BUZZER_PIN_B, LOW);
    
    playStartBeep();
    printHelp();
}

void loop() {
    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        
        if (cmd == "s" || cmd == "S") {
            runScan();
        }
        else if (cmd == "h" || cmd == "H") {
            printHelp();
        }
        else if (cmd.startsWith("f") || cmd.startsWith("F")) {
            float hz = cmd.substring(1).toFloat();
            if (hz > 0 && hz <= 20000) {
                testSingleFreq(hz);
            } else {
                Serial.println("Geçersiz frekans! (1-20000 Hz arası)");
            }
        }
        else if (cmd.startsWith("r") || cmd.startsWith("R")) {
            float hz = cmd.substring(1).toFloat();
            if (hz > 0) {
                peakHz = hz;
                Serial.print("Rezonans frekansı ayarlandı: ");
                Serial.print(hz, 1);
                Serial.println(" Hz");
                printInductanceResult(hz);
            }
        }
        else if (cmd.startsWith("c") || cmd.startsWith("C")) {
            float uf = cmd.substring(1).toFloat();
            if (uf > 0) {
                CAPACITOR_FARADS = uf * 1e-6;
                Serial.print("Kapasitör değeri ayarlandı: ");
                Serial.print(uf, 1);
                Serial.println(" µF");
                
                if (peakHz > 0) {
                    printInductanceResult(peakHz);
                }
            }
        }
        else if (cmd == "x" || cmd == "X") {
            isScanning = false;
            stopTimer();
            Serial.println("⏹ Durduruldu.");
        }
    }
    
    delay(10);
}
