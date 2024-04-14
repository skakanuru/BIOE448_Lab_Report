#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Screen pins
#define TFT_CS   10
#define TFT_RST  9
#define TFT_DC   8

// Button pins
#define BUTTON_SHAKESPEARE_PIN 7
#define BUTTON_TAYLOR_PIN 6

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

void setup() {
  Serial.begin(9600);
  
  pinMode(BUTTON_SHAKESPEARE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_TAYLOR_PIN, INPUT_PULLUP);
  
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  
  displayIntro();
}

void loop() {
  static unsigned long lastQuoteTime = 0;
  static int quoteIndex = -1;
  static bool isShakespeare = true;

  if (millis() - lastQuoteTime > 10000 || quoteIndex == -1) { // Every 10 seconds or first run
    quoteIndex = random(0, 2); // Assuming at least one quote in each array for simplicity
    isShakespeare = random(0, 2);
    displayQuote(quoteIndex, isShakespeare);
    lastQuoteTime = millis();
  }

  checkButtons(isShakespeare);
}

void displayIntro() {
  tft.setCursor(0, 0);
  tft.println("Shakespeare or Taylor Swift?");
  tft.println("Press the corresponding button.");
}

void displayQuote(int index, bool isShakespeare) {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(0, 0);
  if (isShakespeare) {
    tft.println(shakespeareQuotes[index]);
  } else {
    tft.println(taylorSwiftQuotes[index]);
  }
}

void checkButtons(bool isShakespeare) {
  if (digitalRead(BUTTON_SHAKESPEARE_PIN) == LOW) {
    if (isShakespeare) {
      displayCorrect();
    } else {
      displayIncorrect();
    }
  }
  if (digitalRead(BUTTON_TAYLOR_PIN) == LOW) {
    if (!isShakespeare) {
      displayCorrect();
    } else {
      displayIncorrect();
    }
  }
}

void displayCorrect() {
  tft.fillScreen(ILI9341_GREEN);
  tft.setCursor(0, 0);
  tft.println("Correct!");
  delay(2000); // Show result for 2 seconds
  displayIntro();
}

void displayIncorrect() {
  tft.fillScreen(ILI9341_RED);
  tft.setCursor(0, 0);
  tft.println("Incorrect!");
  delay(2000); // Show result for 2 seconds
  displayIntro();
}
