/* 
 * 1088BS LED MATRIX KONTROL SİSTEMİ
 * Göz kırpma animasyonu ve otomatik reset özellikli
 * 
 * Donanım Bağlantıları:
 * - 1088BS Common Cathode 8x8 LED Matrix
 * - Satırlar (Rows): D2-D9
 * - Sütunlar (Columns): D10-D13, A0-A3

 * - Made By Mistytees
 */

// Satır (Row) pin tanımları
#define ROW_1 2  // 1. Satır pini (D2)
#define ROW_2 3  // 2. Satır pini (D3)
#define ROW_3 4  // 3. Satır pini (D4)
#define ROW_4 5  // 4. Satır pini (D5)
#define ROW_5 6  // 5. Satır pini (D6)
#define ROW_6 7  // 6. Satır pini (D7)
#define ROW_7 8  // 7. Satır pini (D8)
#define ROW_8 9  // 8. Satır pini (D9)

// Sütun (Column) pin tanımları
#define COL_1 10  // 1. Sütun pini (D10)
#define COL_2 11  // 2. Sütun pini (D11)
#define COL_3 12  // 3. Sütun pini (D12)
#define COL_4 13  // 4. Sütun pini (D13)
#define COL_5 A0  // 5. Sütun pini (A0)
#define COL_6 A1  // 6. Sütun pini (A1)
#define COL_7 A2  // 7. Sütun pini (A2)
#define COL_8 A3  // 8. Sütun pini (A3)

// Satır ve sütun pin dizileri
const byte R[] = {ROW_1, ROW_2, ROW_3, ROW_4, ROW_5, ROW_6, ROW_7, ROW_8};
const byte C[] = {COL_8, COL_7, COL_6, COL_5, COL_4, COL_3, COL_2, COL_1}; // Ters sıralı bağlantı

// Sistem Değişkenleri
unsigned long resetBaslangicZamani = 0;       // Reset zamanlayıcısı
const unsigned long resetAraligi = 1800000;   // 30 dakika (1,800,000 ms)
unsigned long baslangicZamani = 0;            // Animasyon zamanlayıcısı
unsigned long beklemeSuresi = 0;              // Göz kırpma aralığı (ms)
bool kirpmaAsamasinda = false;                // Animasyon durum flag'i

/* 
 * GÖZ ANİMASYON DESENLERİ 
 * 8x8 boyutunda, 1=LED AÇIK, 0=LED KAPALI
 * 
 * 1088BS Matris Özellikleri:
 * - Common Cathode yapı
 * - Maksimum 20mA/sütun akımı
 * - 2.0-2.5V LED ileri voltajı
 */
unsigned char eyeOpen[8][8] = {
  {0,0,0,0,0,0,0,0},    // 1. satır
  {0,0,1,1,1,1,0,0},    // 2. satır - üst göz kapağı
  {0,1,0,0,0,0,1,0},    // 3. satır - gözün üst kısmı
  {1,0,0,1,1,0,0,1},    // 4. satır - iris (göz bebeği)
  {1,0,0,1,1,0,0,1},    // 5. satır - iris (göz bebeği)
  {0,1,0,0,0,0,1,0},    // 6. satır - gözün alt kısmı
  {0,0,1,1,1,1,0,0},    // 7. satır - alt göz kapağı
  {0,0,0,0,0,0,0,0}     // 8. satır
};

unsigned char eyeHalfOpen[8][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},    // Üst göz kapağı yarı kapalı
  {0,0,1,1,1,1,0,0},    // Gözün daraltılmış hali
  {1,1,0,1,1,0,1,1},    // Yarım görünen iris
  {1,1,0,1,1,0,1,1},    // Yarım görünen iris
  {0,0,1,1,1,1,0,0},    // Gözün daraltılmış hali
  {0,0,0,0,0,0,0,0},    // Alt göz kapağı yarı kapalı
  {0,0,0,0,0,0,0,0}
};

unsigned char eyeClosed[8][8] = {
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {1,1,1,1,1,1,1,1},    // Tam kapalı göz çizgisi
  {1,1,1,1,1,1,1,1},    // Tam kapalı göz çizgisi
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0}
};

// DEBUG için tüm LED'leri açan desen
unsigned char debug[8][8] = {
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1},
  {1,1,1,1,1,1,1,1}
};

void setup() {
  // LED matris pinlerini ayarla
  for(int i = 0; i < 8; i++) {
    pinMode(R[i], OUTPUT);  // Tüm satır pinleri çıkış
    pinMode(C[i], OUTPUT);  // Tüm sütun pinleri çıkış
    digitalWrite(R[i], LOW); // Başlangıçta satırları kapat
    digitalWrite(C[i], HIGH); // Sütunları pasif yap
  }
  
  // Seri iletişimi başlat (9600 baud)
  Serial.begin(9600);
  Serial.println(F("1088BS LED Matrix Kontrol Sistemi"));
  Serial.println(F("Göz kırpma animasyonu aktif"));
  
  // Rastgele sayı üreteci için seed ayarla
  randomSeed(analogRead(A4)); // Analog 4. pinden gürültü okuma
  
  // İlk göz kırpma aralığını rastgele ayarla (1-15 saniye)
  beklemeSuresi = random(1000, 15000);
  baslangicZamani = millis(); // Animasyon zamanlayıcısını başlat
  resetBaslangicZamani = millis(); // Reset zamanlayıcısını başlat
}

void loop() {
  /* RESET GERİ SAYIM HESAPLAMA */
  unsigned long resetKalanSure = resetAraligi - (millis() - resetBaslangicZamani);
  
  /*
   * Zaman dönüşümü:
   * 1. Milisaniyeyi 60000'e böl -> dakika
   * 2. Milisaniyenin 60000'e bölümünden kalanı 1000'e böl -> saniye
   * Örnek: 80000ms -> 80000/60000=1dk, 80000%60000=20000/1000=20sn
   */
  int resetKalanDakika = resetKalanSure / 60000;
  int resetKalanSaniye = (resetKalanSure % 60000) / 1000;

  /* ANİMASYON KONTROLÜ */
  if (!kirpmaAsamasinda) {
    // NORMAL MOD (Göz açık)
    Display(eyeOpen);
    
    // Göz kırpma zamanı kontrolü
    if (millis() - baslangicZamani >= beklemeSuresi) {
      kirpmaAsamasinda = true;
      Serial.println(F("Göz kırpma animasyonu başlıyor..."));
      baslangicZamani = millis(); // Animasyon zamanlayıcısını sıfırla
    }
    
    // 5 saniyede bir durum raporu
    static unsigned long sonRaporZamani = 0;
    if (millis() - sonRaporZamani >= 5000) {
      sonRaporZamani = millis();
      
      // Sonraki kırpmaya kalan süre
      float kirpmaKalanSure = (beklemeSuresi - (millis() - baslangicZamani)) / 1000.0;
      
      Serial.print(F("Sonraki kırpma: "));
      Serial.print(kirpmaKalanSure, 1); // 1 ondalık basamak
      Serial.print(F(" sn | "));
      
      // Reset için kalan zaman
      Serial.print(F("Reset: "));
      Serial.print(resetKalanDakika);
      Serial.print(F(":"));
      if(resetKalanSaniye < 10) Serial.print('0'); // 04, 09 gibi
      Serial.print(resetKalanSaniye);
      Serial.println(F(" (dk:sn)"));
    }
  } 
  else {
    /* ANİMASYON AŞAMALARI */
    if (millis() - baslangicZamani < 100) {
      // 1. Aşama: Yarı açık (100ms)
      Display(eyeHalfOpen);
    } 
    else if (millis() - baslangicZamani < 200) {
      // 2. Aşama: Kapalı (100ms)
      Display(eyeClosed);
    }
    else if (millis() - baslangicZamani < 300) {
      // 3. Aşama: Yarı açık (100ms)
      Display(eyeHalfOpen);
    }
    else {
      // ANİMASYON TAMAMLANDI
      kirpmaAsamasinda = false;
      beklemeSuresi = random(1000, 15000); // Yeni bekleme süresi
      baslangicZamani = millis();
      
      // Bilgilendirme mesajı
      Serial.print(F("Animasyon tamamlandı. Sonraki kırpma: "));
      Serial.print(beklemeSuresi/1000.0, 1);
      Serial.println(F(" sn sonra"));
    }
  }

  /* OTOMATİK RESET KONTROLÜ (30 dakikada bir) */
  if (millis() - resetBaslangicZamani >= resetAraligi) {
    Serial.println(F("30 dakika doldu, sistem resetleniyor..."));
    delay(100); // Seri iletişimin tamamlanmasını bekle
    
    // Yazılımsal reset
    void(* resetFunc) (void) = 0; // Reset fonksiyon pointer'ı
    resetFunc(); // Programı baştan başlat
  }
}

/**
 * @brief LED matrisine desen çizer
 * @param dat[8][8] Çizilecek 8x8 desen (1=Açık, 0=Kapalı)
 * @details Sütun tarama yöntemi kullanır:
 * 1. Bir sütunu aktif eder (LOW)
 * 2. O sütundaki satırları kontrol eder
 * 3. 1ms bekler (POV etkisi için)
 * 4. Tüm LED'leri kapatır
 * 5. Sonraki sütuna geçer
 */
void Display(unsigned char dat[8][8]) {
  for(int c = 0; c < 8; c++) {
    digitalWrite(C[c], LOW); // Sütunu aktif et
    
    // Satır verilerini yaz
    for(int r = 0; r < 8; r++) {
      digitalWrite(R[r], dat[r][c] ? HIGH : LOW);
    }
    
    delay(1); // Görüntüleme süresi
    Clear();  // Ghosting önleme
  }
}

/**
 * @brief Tüm LED'leri kapatır
 * @details 
 * - Tüm satırları LOW yaparak LED'leri kapatır
 * - Tüm sütunları HIGH yaparak sütunları devre dışı bırakır
 * - Enerji tasarrufu ve hayalet görüntüyü önler
 */
void Clear() {
  for(int i = 0; i < 8; i++) {
    digitalWrite(R[i], LOW);
    digitalWrite(C[i], HIGH);
  }
}
