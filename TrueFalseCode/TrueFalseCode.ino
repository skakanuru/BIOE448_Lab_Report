#include <WiFiNINA.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

// Screen and button pins
#define TFT_CS   4
#define TFT_RST  6
#define TFT_DC   7
#define BUTTON_NEXT_PIN 2
#define BUTTON_SELECT_PIN 5

// Display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// WiFi credentials
char ssid[] = "vale";        // your network SSID (name)
char pass[] = "valele13";    // your network password
int status = WL_IDLE_STATUS;

// Cloud server details
const char* serverAddress = "172.20.10.10";
const int serverPort = 80;

WiFiClient client;

// Cloud properties
String playerInitials;
int scoreboard;

const char* shakespeareQuotes[] = {
  "To be, or not to be: that is the question.",
  "All the world's a stage, and all the men and women merely players.",
  "Romeo, Romeo, wherefore art thou Romeo?",
  // ... add more Shakespeare quotes
};

const char* taylorSwiftQuotes[] = {
  "I'm feeling 22!",
  "Haters gonna hate, hate, hate, hate, hate.",
  "I knew you were trouble when you walked in.",
  // ... add more Taylor Swift quotes
};

// State variables
enum GameState { INTRO, DISPLAY_QUOTE, WAIT_FOR_INPUT, SHOW_RESULT, GAME_OVER };
GameState currentState = INTRO;

unsigned long startTime = 0;
unsigned long lastActionTime = 0;
int score = 0;
bool isShakespeare = true;
bool lastResult = false;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }

  // Connect to Wi-Fi
  connectToWiFi();

  pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP);

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
    startTime = millis();
    displayRandomQuote();
  }

  if (currentState == WAIT_FOR_INPUT) {
    if (millis() - startTime > 15000) {
      currentState = GAME_OVER;
      displayGameOver();
    } else {
      checkButtons();
    }
  }

  if (currentState == SHOW_RESULT && millis() - lastActionTime > 2000) {
    isShakespeare = random(0, 2);
    displayRandomQuote();
  }
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void displayIntro() {
  tft.setCursor(0, 0);
  tft.println("Welcome to the Game!");
  tft.println("Press start when ready.");
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
  if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
    lastResult = isShakespeare;
    currentState = SHOW_RESULT;
    displayResult(lastResult);
  }
  if (digitalRead(BUTTON_SELECT_PIN) == LOW) {
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
  lastActionTime = millis();
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
  tft.println(score);

  getUserInitials();
  updateScoreboard();

  tft.setCursor(0, 90);
  tft.println("Press any button to restart.");
  waitForButtonPress();
  resetGame();
}

void getUserInitials() {
  char initials[4] = "AAA";
  int position = 0;

  tft.fillRect(0, 60, tft.width(), 30, ILI9341_BLACK);
  tft.setCursor(0, 60);
  tft.println("Enter initials:");
  tft.setCursor(0, 80);
  tft.print("AAA");

  while (position < 3) {
    if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
      initials[position]++;
      if (initials[position] > 'Z') initials[position] = 'A';
      tft.fillRect(position * 20, 80, 20, 20, ILI9341_BLACK);
      tft.setCursor(position * 20, 80);
      tft.print(initials[position]);
      delay(200);
    }

    if (digitalRead(BUTTON_SELECT_PIN) == LOW) {
      position++;
      delay(200);
    }

    Serial.print("Current initials: ");
    for (int i = 0; i < position; i++) {
      Serial.print(initials[i]);
    }
    Serial.println();
  }

  initials[3] = '\0';
  playerInitials = initials;

  Serial.print("Selected initials: ");
  Serial.println(initials);
}

void updateScoreboard() {
  String leaderboardData = "initials=" + playerInitials + "&score=" + String(score);

  if (client.connect(serverAddress, serverPort)) {
    client.println("POST /leaderboard HTTP/1.1");
    client.println("Host: " + String(serverAddress));
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(leaderboardData.length());
    client.println();
    client.print(leaderboardData);

    while (client.connected()) {
      if (client.available()) {
        String response = client.readStringUntil('\n');
        Serial.println("Server response: " + response);
        break;
      }
    }

    client.stop();
  } else {
    Serial.println("Connection to server failed");
  }

  Serial.print("Scoreboard updated: ");
  Serial.print(playerInitials);
  Serial.print(" - ");
  Serial.println(score);
}

void waitForButtonPress() {
  while (digitalRead(BUTTON_NEXT_PIN) == HIGH && digitalRead(BUTTON_SELECT_PIN) == HIGH) {
    // Wait for any button to be pressed
  }
  delay(200);
}

void resetGame() {
  score = 0;
  currentState = INTRO;
  displayIntro();
}