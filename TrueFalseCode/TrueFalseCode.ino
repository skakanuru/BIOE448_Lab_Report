#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Screen pins
#define TFT_CS   4
#define TFT_RST  6
#define TFT_DC   7

// Button pins
#define BUTTON_SHAKESPEARE_PIN 1
#define BUTTON_TAYLOR_PIN 2

// Create display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Quotes arrays
const char* shakespeareQuotes[] = {
  "My tongue will tell the anger of my heart, or else my heart concealing it will break",
  // Add more quotes
};

const char* taylorSwiftQuotes[] = {
  "Did I close my fist around something delicate? Did I shatter you?",
  // Add more quotes
};

// State variables
enum GameState { INTRO, DISPLAY_QUOTE, WAIT_FOR_INPUT, SHOW_RESULT, GAME_OVER };
GameState currentState = INTRO;

unsigned long startTime = 0;
unsigned long lastActionTime = 0;
int score = 0;
bool isShakespeare = true;
bool lastResult = false; // True if correct, false if incorrect

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_SHAKESPEARE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_TAYLOR_PIN, INPUT_PULLUP);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  displayIntro();
}

void loop() {
  if (currentState == INTRO && millis() - lastActionTime > 5000) {
    currentState = DISPLAY_QUOTE;
    startTime = millis();  // Start the game timer
    displayRandomQuote();
  }

  if (currentState == WAIT_FOR_INPUT) {
    if (millis() - startTime > 60000) {  // 60 seconds have passed
      currentState = GAME_OVER;
      displayGameOver();
    } else {
      checkButtons();
    }
  }

  if (currentState == SHOW_RESULT && millis() - lastActionTime > 2000) {
    isShakespeare = random(0, 2); // Randomly decide if next quote is Shakespeare
    displayRandomQuote();
  }
}

void displayIntro() {
  tft.setCursor(0, 0);
  tft.println("Shakespeare or Taylor Swift?");
  tft.println("Press the corresponding button.");
  lastActionTime = millis();
}

void displayRandomQuote() {
  int quoteIndex = 0;
  if (isShakespeare) {
    quoteIndex = random(0, sizeof(shakespeareQuotes) / sizeof(shakespeareQuotes[0]));
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.println(shakespeareQuotes[quoteIndex]);
  } else {
    quoteIndex = random(0, sizeof(taylorSwiftQuotes) / sizeof(taylorSwiftQuotes[0]));
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(0, 0);
    tft.println(taylorSwiftQuotes[quoteIndex]);
  }
  currentState = WAIT_FOR_INPUT;
}

void checkButtons() {
  if (digitalRead(BUTTON_SHAKESPEARE_PIN) == LOW) {
    lastResult = isShakespeare;
    currentState = SHOW_RESULT;
    displayResult(lastResult);
  }
  if (digitalRead(BUTTON_TAYLOR_PIN) == LOW) {
    lastResult = !isShakespeare;
    currentState = SHOW_RESULT;
    displayResult(lastResult);
  }
}

void displayResult(bool correct) {
  if (correct) {
    score++;
    displayCorrect();
  } else {
    displayIncorrect();
  }
  lastActionTime = millis(); // Reset timer for result display
}

void displayCorrect() {
  tft.fillScreen(ILI9341_GREEN);
  tft.setCursor(0, 0);
  tft.println("Correct!");
}

void displayIncorrect() {
  tft.fillScreen(ILI9341_RED);
  tft.setCursor(0, 0);
  tft.println("Incorrect!");
}

void displayGameOver() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.println("Game Over!");
  tft.setCursor(0, 30);
  tft.print("Your Score: ");
  tft.print(score);
  // Optionally restart or wait for a button press to restart
}
