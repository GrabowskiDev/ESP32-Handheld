# Sprawozdanie z projektu ESP32-Handheld

## 1. Wstęp
Projekt **ESP32-Handheld** to przenośna konsola do gier w stylu klasycznego GameBoya. Celem projektu jest stworzenie platformy pozwalającej na uruchamianie prostych gier i aplikacji napisanych w języku C++. Całość opiera się o architekturę mikrokontrolera ESP32, co zapewnia wystarczającą moc obliczeniową do płynnej obsługi kolorowego wyświetlacza, interfejsu przycisków oraz odtwarzania dźwięku.

### Wykorzystane komponenty i technologie:
* **Mikrokontroler:** ESP32-S3-DEV-KIT-N8R8 - główne serce układu, odpowiedzialne za wykonywanie logiki gier oraz komunikację i sterowanie sprzętem.
* **Oprogramowanie i Język:** Cały kod napisany jest w **C++**, z wykorzystaniem frameworka **Arduino** i wbudowany na platformie deweloperskiej **PlatformIO**.
* **Ekran:** Kolorowy wyświetlacz o rozdzielczości 240x320 pikseli komunikujący się z mikrokontrolerem po szybkiej magistrali SPI.
* **System Audio:** Oparty o układ MAX98357A (wzmacniacz klasy D) wykorzystujący do komunikacji protokół I2S, podłączony do głośnika o impedancji 4Ω.
* **Zasilanie:** Przenośność jest zapewniona dzięki wykorzystaniu akumulatora Li-Pol 2200mAh. Akumulator jest obsługiwany przez układ ładowania TP4056 i połączony z przetwornicą Step-Up (Boost), podnoszącą napięcie do 5V wymaganego przez system.
* **Interfejs użytkownika:** Autorska "button board" (płytka przycisków), na której znajduje się układ sterowania kierunkowego (góra, dół, lewo, prawo) oraz klasyczne przyciski funkcyjne: A, B, Start i Select.

## 2. Schemat
Architektura sprzętowa projektu, zgodnie z dołączonym do niego schematem, dzieli się na kilka współpracujących ze sobą, lecz funkcjonalnie niezależnych systemów. Punktem zbiorczym dla całości jest mikrokontroler ESP32:
* **System zasilania (Power):** Obwód zapewniający stałe i bezpieczne zasilanie dla elektroniki. Napięcie z akumulatora przechodzi przez układ ładujący (TP4056), główny włącznik zasilania (Switch) i przetwornicę 5V, by na końcu zasilić układ ESP32.
* **Wyświetlacz (Display):** Podłączony do pinów mikrokontrolera przeznaczonych dla protokołu SPI, co umożliwia wysyłanie klatek wideo z dużą prędkością.
* **System dźwiękowy (Audio System):** Wykorzystuje specjalistyczne piny I2S w mikrokontrolerze ESP32, aby wysyłać strumień dźwięku (audio cyfrowe) do zewnętrznego wzmacniacza sygnału przed wyprowadzeniem go na głośnik.
* **Interfejs wejścia (Input):** Zestaw fizycznych przycisków bezpośrednio zmapowanych do portów GPIO (wejścia/wyjścia) na ESP32. Zamknięcie obwodu danego przycisku do masy (GND) informuje układ o wykonanej akcji.

Całość jest zaprojektowana tak, aby komponenty mogły ze sobą bezproblemowo współpracować, korzystając z odseparowanych interfejsów komunikacyjnych ESP32.

## 3. Najważniejsze miejsca w kodzie

Z punktu widzenia oprogramowania, projekt charakteryzuje się nowoczesnym i przejrzystym podziałem pomiędzy warstwą sprzętową (urządzeniami), warstwą rdzenia (silnikiem zarządzającym), a samymi aplikacjami/grami. Poniżej przedstawiono kluczowe elementy realizujące oprogramowanie samej konsoli (z pominięciem niskopoziomowych sterowników sprzętowych).

### Interfejs gier i aplikacji - `IGame` (`include/core/IGame.h`)
Podstawowym budulcem całego systemu jest czysto wirtualna klasa bazowa `IGame`. Wymusza ona na każdym module (który ma działać jako gra) zaimplementowanie rygorystycznego cyklu życia. Interfejs ten definiuje cztery główne metody:
```cpp
class IGame {
public:
    virtual ~IGame() {}
    virtual bool isMainMenu() const { return false; }

    virtual void start() = 0;   // Wywoływana jednorazowo - alokacja pamięci, grafik itp.
    virtual void update() = 0;  // Wywoływana co klatkę - logika, wejście (input), fizyka
    virtual void draw() = 0;    // Wywoływana co klatkę - renderowanie klatki na ekran
    virtual void stop() = 0;    // Wywoływana przed zniszczeniem - zwalnianie zasobów
};
```

### Serce systemu - `GameManager` (`src/core/GameManager.cpp`)
Menedżer Gry (klasa `GameManager`) zarządza cyklem przełączania widoków na konsoli. Przechowuje on informację (wskaźnik) o tym, jaka gra lub aplikacja znajduje się w pamięci na pierwszym planie (`currentGame`). 
Menedżer dysponuje funkcją `switchGame(IGame* newGame)`, która bezpiecznie i bez wycieków pamięci podmienia aktualnie działającą grę:
```cpp
void GameManager::switchGame(IGame* newGame) {
    if (currentGame) {
        currentGame->stop();
        // Ponieważ przekazujemy wskaźniki, GameManager zarządza pamięcią i zwalnia obiekt
        delete currentGame; 
    }
    
    currentGame = newGame;
    
    if (currentGame) {
        currentGame->start();
    }
}
```
Dzięki temu konsola może obsługiwać dowolnie złożone aplikacje przy zachowaniu restrykcyjnej gospodarki pamięcią ESP32.

### Pętla główna programu (`src/main.cpp`)
Plik ten to most między sprzętem a środowiskiem obiektowym systemu konsoli. W funkcji `setup()` (wywoływanej po starcie) inicjalizowany jest sprzęt i uruchamiane jest Menu:
```cpp
void setup() {
    Serial.begin(115200);
    // Inicjalizacja sprzętowa
    Display::setup();
    Buttons::setup();
    Sound::setup();
    Sound::startTask();

    // Przekazanie do zarządzania pierwszej "Gry", czyli Menu Głównego
    gameManager.switchGame(new MainMenu());
}
```
Funkcja `loop()`, działająca niczym silnik gier (ok. 60 iteracji na sekundę), wykonuje stałą rutynę:
1. **Odświeżenie wejścia:** Wczytuje aktualne stany ze sterownika przycisków.
2. **Globalne wyjście:** Sprawdza obecność tak zwanego "Global Exit Chord". Naciśnięcie `SELECT` i `START` natychmiast wymusza powrót do Menu.
3. **Cykl życia logiki i grafiki:** Przekazuje kontrolę w głąb aktywnej gry:
```cpp
void loop() {
    Buttons::update();

    // ... (kod sprawdzający m.in. SELECT+START dla awaryjnego wyjścia z gry) ...

    gameManager.update(); // 1. Zaktualizuj stan gry
    gameManager.draw();   // 2. Narysuj nową klatkę
    Display::update();    // 3. Wyślij odrysowaną klatkę do fizycznego wyświetlacza

    delay(16); // Limit opóźnienia do ~60 klatek na sekundę
}
```

### Aplikacja systemowa - `MainMenu` (`src/games/MainMenu.cpp`)
Aplikacja Menu (dziedzicząca po `IGame`) pozwala na nawigację po dostępnej liście wbudowanych gier. Odczytuje wychylenia "krzyżaka" zmieniając zaznaczenie listy w `update()`, oraz rysuje interfejs w `draw()`. Zatwierdzenie wyboru powoduje stworzenie nowego obiektu wybranej gry (np. `new TetrisGame()`) i wymuszenie jego załadowania przez `GameManager`.

## 4. Przykładowa implementacja gry: Tetris

Znakomitym przykładem wykorzystania przygotowanej architektury jest działająca na konsoli gra **Tetris** (`src/games/TetrisGame.cpp`). W jej kodzie zastosowano kilka zoptymalizowanych zabiegów przydatnych w programowaniu mikrokontrolerów:

* **Pamięciowa Reprezentacja Klocków (Tetromino):** W przeciwieństwie do tworzenia dużych obiektów, układów współrzędnych czy dwuwymiarowych tablic w pamięci RAM, każdy ze stanów rotacji klocka zapisany jest w systemie heksadecymalnym jako stała `uint16_t` (pojedyncza 16-bitowa liczba całkowita). Traktowana jest ona jak siatka bitów 4x4:
```cpp
static const uint16_t SHAPES[7][4] = {
    {0x0F00, 0x2222, 0x00F0, 0x4444}, // Kształt I (Linia) w 4 rotacjach
    {0x44C0, 0x8E00, 0x6440, 0x0E20}, // Kształt J
    // ... (pozostałe kształty O, L, T, S, Z)
};
```
Oszczędza to ogromne ilości cennej pamięci Flash/RAM. Detekcja kolizji wykorzystuje przesunięcia bitowe (`shape & (0x8000 >> i)`), pozwalając procesorowi na ułamkowo szybkie przetworzenie czy dany klocek styka się z elementami planszy.

* **Nowoczesne mechaniki Tetrisa:** Oprogramowanie gry rozszerzono o zasady popularne we współczesnych wariacjach tej produkcji. Mamy tutaj podgląd upadku zwany **"Ghost Block"** – pętla symuluje upadek i obrysowuje w miejscu uderzenia półprzezroczysty kwadrat jako ułatwienie dla gracza. Zaimplementowano także wsparcie do przetrzymywania bloków w pamięci na później poprzez wciśnięcie klawisza `SELECT` (Hold).
* **Profesjonalny Input Gracza:** Tetris implementuje rozwiązanie **DAS** (Delayed Auto Shift). Oznacza to, że pojedyncze wciśnięcie gałki w bok przesuwa klocek o 1 pole, lecz jej przytrzymanie czeka najpierw np. 250ms, a potem płynnie wprawia blok w kontrolowany ślizg w lewo lub w prawo, zwiększając zręcznościowe odczucia z obsługi.
