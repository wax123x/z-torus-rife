# AGENTS.md - Donanım ve Yazılım Kural Kılavuzu

## 1. Donanımsal Öncelik Kuralı (Physical First Principle)
- Bir donanım veya sinyal hatası yaşandığında; karmaşık yazılım, çip veya JTAG senaryoları üretmeden ÖNCE her zaman fiziki kontrolleri (kablo temassızlığı, yanlış pin numarası, ortak toprak (GND) ve 5V/VCC beslemesi) öncelikli olarak sorgula.

## 2. Kanıtsız Teorik Teşhis Yasağı (No Unverified Guesses)
- Kodda, şemada veya datasheet'te somut kanıtı olmadan hiçbir pini veya entegreyi "kilitli, arızalı veya uyumsuz" ilan etme. 

## 3. Kesin Sebep-Sonuç Mantığı (Strict Logic & Cause-Effect)
- Güç, voltaj, frekans, duty cycle ve manyetik alan hesabı yaparken matematiksel sebep-sonuç ilişkilerini eksiksiz kur. Birbiriyle çelişen ifadeler kullanma.

## 4. Donanım ve Entegre Datasheet Doğrulama Kuralı (Strict Semiconductor & Datasheet Verification)
- Kullanıcı bir çip, modül veya sürücü kartı gösterdiğinde veya sorduğunda, "tam çözüm", "harika" gibi yüzeysel ifadeler kullanmadan ÖNCE:
  1. Entegrenin iç yarı iletken teknolojisini (BJT Darlington, MOSFET, IGBT, Class-D) datasheet üzerinden sorgula.
  2. Kullanıcının hedef frekans aralığındaki (1 Hz - 20.000 Hz) anahtarlama hız sınırlarını (f_max) ve kayıplarını kontrol et.
  3. Datasheet'te frekans veya güç kısıtı varsa, bunu kullanıcıya kartı bağlatmadan ÖNCE bildir. Doğrulanmamış hiçbir karta "kesin çözüm" deme!
