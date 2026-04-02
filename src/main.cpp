// #include <Arduino.h>

// #define LED_PIN 38

// // put function declarations here:
// int myFunction(int, int);

// void setup() {
//   // put your setup code here, to run once:
//   int result = myFunction(2, 3);
//   Serial.begin(115200);
//   delay(3000); // Dajemy portowi szeregowemu 1 sekundę na "rozgrzanie się"
//   Serial.print("Wynik funkcji myFunction(2, 3) = ");
//   Serial.println(result);
//   pinMode(LED_PIN, OUTPUT);
//   digitalWrite(LED_PIN, HIGH);
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   Serial.println("hejka");
//   //rgb light
//   delay(100);

// }

// // put function definitions here:
// int myFunction(int x, int y) {
//   return x + y;
// }





// // DISPLAY TEST
// #include <Arduino.h>
// #include <TFT_eSPI.h> 

// // Główny obiekt ekranu
// TFT_eSPI tft = TFT_eSPI(); 

// // Obiekt Sprite (przekazujemy mu wskaźnik do głównego ekranu)
// TFT_eSprite mySprite = TFT_eSprite(&tft); 

// int counter = 0;

// void setup() {
//   Serial.begin(115200);
  
//   tft.init();
//   tft.setRotation(1); // Orientacja pozioma
//   tft.fillScreen(TFT_BLACK);
  
//   // Rysujemy statyczny tekst bezpośrednio na ekranie (zrobi się to tylko raz)
//   tft.setTextColor(TFT_CYAN);
//   tft.setTextSize(2);
//   tft.setCursor(10, 10);
//   tft.println("Plynny licznik:");

//   // --- Konfiguracja Sprite'a ---
//   // Tworzymy bufor w RAM o rozmiarze: szerokość 120px, wysokość 40px
//   // ESP32 ma dużo pamięci, więc taki rozmiar to żaden problem
//   mySprite.createSprite(120, 40); 
  
//   // Konfigurujemy tekst wewnątrz Sprite'a
//   mySprite.setTextSize(3);
//   // Ważne: ustawiamy kolor tekstu ORAZ kolor tła. 
//   // Pomaga to w wygładzaniu krawędzi (antyaliasingu).
//   mySprite.setTextColor(TFT_GREEN, TFT_BLACK); 
// }

// void loop() {
//   // 1. Czyścimy TYLKO naszego Sprite'a w pamięci RAM (nie cały ekran!)
//   mySprite.fillSprite(TFT_BLACK);
  
//   // 2. Rysujemy i piszemy wewnątrz Sprite'a.
//   // UWAGA: Współrzędne tutaj (x=5, y=10) są względem LEWEGO GÓRNEGO ROGU SPRITE'A, 
//   // a nie całego ekranu!
//   mySprite.setCursor(5, 10);
//   mySprite.print(counter);
  
//   // 3. Magia! Wypychamy gotowy, narysowany bufor na fizyczny ekran.
//   // Tutaj podajemy współrzędne ekranu (x=10, y=40), gdzie ma się pojawić nasz Sprite.
//   mySprite.pushSprite(10, 40);
  
//   counter++;
  
//   // Małe opóźnienie, abyśmy zdążyli zauważyć zmieniające się liczby
//   delay(50); 
// }



// // ============== SOUND TEST ====================
// 
// #include <Arduino.h>
// #include "sound.h"

// void setup() {
//     Serial.begin(115200);
//     // mpuSensor.setup();
//     Sound::setup();
//     Sound::startTask();
//     // neoPixel.updateBladeColor(); // Initialize to current color mode
// }

// void loop() {
//   Sound::play(SOUND_IGNITE);
//   delay(1000); 
// }


// #define BTN1_PIN 25
// #define BTN2_PIN 26
// #define BTN3_PIN 27
// #define BTN4_PIN 32

// void setup() {
//   Serial.begin(115200);

//   // Konfiguracja pinów jako wejścia z podciągnięciem do zasilania (VCC)
//   pinMode(BTN1_PIN, INPUT_PULLUP);
//   pinMode(BTN2_PIN, INPUT_PULLUP);
//   pinMode(BTN3_PIN, INPUT_PULLUP);
//   pinMode(BTN4_PIN, INPUT_PULLUP);
  
//   Serial.println("System gotowy. Czekam na wcisniecie...");
// }

// void loop() {
//   // PAMIĘTAJ: LOW oznacza wciśnięty, HIGH oznacza zwolniony
  
//   if (digitalRead(BTN1_PIN) == LOW) {
//     Serial.println("Wcisnieto Przycisk 1!");
//     delay(200); // Prosty debouncing (wyjaśnienie poniżej)
//   }
  
//   if (digitalRead(BTN2_PIN) == LOW) {
//     Serial.println("Wcisnieto Przycisk 2!");
//     delay(200); 
//   }

//   if (digitalRead(BTN3_PIN) == LOW) {
//     Serial.println("Wcisnieto Przycisk 3!");
//     delay(200); 
//   }

//   if (digitalRead(BTN4_PIN) == LOW) {
//     Serial.println("Wcisnieto Przycisk 4!");
//     delay(200); 
//   }
// }


// #include <Arduino.h>
// #include <TFT_eSPI.h> 

// TFT_eSPI tft = TFT_eSPI(); 

// void setup() {
//   Serial.begin(115200);
  
//   // Dajemy portowi szeregowemu 2 sekundy na "rozgrzanie się", 
//   // abyś zdążył zobaczyć logi przed ewentualnym błędem.
//   delay(2000); 
//   Serial.println("\n\n--- START SYSTEMU ---");

//   Serial.println("Krok 1: Inicjalizacja ekranu...");
//   tft.init();
//   tft.setRotation(1);
  
//   Serial.println("Krok 2: Wypelnienie tla...");
//   // Jeśli to przejdzie, ekran powinien błysnąć na jaskrawy CZERWONY kolor!
//   tft.fillScreen(TFT_RED); 
  
//   Serial.println("Krok 3: Rysowanie tekstu...");
//   tft.setTextColor(TFT_WHITE);
//   tft.setTextSize(3);
//   tft.setCursor(10, 10);
//   tft.println("Test ekranu OK!");
  
//   Serial.println("Krok 4: Wszystko dziala stabilnie!");
// }

// void loop() {
//   // Pusta pętla na czas testów
//   delay(1000);
// }

#include <Arduino.h>
#include <TFT_eSPI.h> // Graphics and font library for ILI9341 driver chip
#include <SPI.h>
#include "sound.h"

#define TFT_GREY 0x5AEB // New colour

#define BTN1_PIN 25
#define BTN2_PIN 26
#define BTN3_PIN 27
#define BTN4_PIN 32

TFT_eSPI tft = TFT_eSPI();  // Invoke library

int testMode = 0;

void testDisplay() {
  tft.fillScreen(TFT_BLUE);
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.println("Display Test");
}

void testButtons() {
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.println("Button Test");
  tft.setCursor(10, 40);
  tft.println("Press buttons...");

  // Check buttons and display
  if (digitalRead(BTN1_PIN) == LOW) {
    tft.setCursor(10, 70);
    tft.println("BTN1 pressed");
  }
  if (digitalRead(BTN2_PIN) == LOW) {
    tft.setCursor(10, 100);
    tft.println("BTN2 pressed");
  }
  if (digitalRead(BTN3_PIN) == LOW) {
    tft.setCursor(10, 130);
    tft.println("BTN3 pressed");
  }
  if (digitalRead(BTN4_PIN) == LOW) {
    tft.setCursor(10, 160);
    tft.println("BTN4 pressed");
  }
}

void testSound() {
  tft.fillScreen(TFT_GREEN);
  tft.setCursor(10, 10);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(2);
  tft.println("Sound Test");
  Sound::play(SOUND_IGNITE);
}

void setup(void) {
    Serial.begin(115200);
    delay(2000); // Dajemy portowi szeregowemu 2 sekundy na "rozgrzanie się"
    Serial.println("\n\n--- START SYSTEMU ---");
  
    Serial.println("Inicjalizacja ekranu...");
  tft.init();
  Serial.println("Ustawianie orientacji...");
  tft.setRotation(2);
  Serial.println("Wypelnianie tla...");

  // Initialize buttons
  // pinMode(BTN1_PIN, INPUT_PULLUP);
  // pinMode(BTN2_PIN, INPUT_PULLUP);
  // pinMode(BTN3_PIN, INPUT_PULLUP);
  // pinMode(BTN4_PIN, INPUT_PULLUP);

  // Initialize sound
  Sound::setup();
  Sound::startTask();
}

void loop() {
  switch(testMode) {
    case 0:
      testDisplay();
      break;
    case 1:
      // testButtons();
      break;
    case 2:
      testSound();
      break;
  }
  testMode = (testMode + 1) % 3;
  delay(5000); // 5 seconds per test
}
