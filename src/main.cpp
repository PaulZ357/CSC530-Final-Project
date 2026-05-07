#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SdFat.h>
#include <IRremote.h>

SdFat SD;
File32 myFile;
String fileName = "file_log.txt";
String UNLOCK_CODE_FILE = "unlock.txt";

const int CORRECT_TONE = 500;
const int INCORRECT_TONE = 1000;
const int speakerPin = D3;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// function declarations
void readSD();
void showLockSymbol();
void showUnlockSymbol();
void playTone(int, int);
void incorrect();

void setup() {
  pinMode(speakerPin, OUTPUT);
  readSD();
}

void loop() {
  // this is just demonstration of lock and unlock as well as buzzer
  showLockSymbol();
  incorrect();
  delay(1000);
  showUnlockSymbol();
  delay(1000);
}

void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

void incorrect() {
  playTone(INCORRECT_TONE, 100);
  delay(100);
  playTone(INCORRECT_TONE, 100);
  delay(100);
}

void showLockSymbol() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  // ---- LOCK ICON ----

  // Lock body
  display.fillRect(50, 30, 28, 20, SSD1306_WHITE);

  // Lock shackle (top curve)
  display.drawCircle(64, 25, 10, SSD1306_WHITE);

  // Turn circle into U shape
  display.fillRect(54, 26, 20, 4, SSD1306_WHITE);
  display.fillRect(55, 26, 19, 4, SSD1306_BLACK);

  // Optional keyhole
  display.fillCircle(64, 38, 2, SSD1306_BLACK);
  display.fillRect(63, 40, 3, 5, SSD1306_BLACK);

  display.display();
}

void showUnlockSymbol() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  // ---- UNLOCK ICON ----

  // Lock body
  display.fillRect(50, 30, 28, 20, SSD1306_WHITE);

  // Lock shackle (top curve)
  display.drawCircle(64 + 20, 25, 10, SSD1306_WHITE);

  // Turn circle into U shape
  display.fillRect(54 + 20, 26, 20, 4, SSD1306_WHITE);
  display.fillRect(55 + 20, 26, 19, 4, SSD1306_BLACK);

  // Optional keyhole
  display.fillCircle(64, 38, 2, SSD1306_BLACK);
  display.fillRect(63, 40, 3, 5, SSD1306_BLACK);

  // UNLOCK part
  display.fillRect(50 + 28, 30, 28, 20, SSD1306_BLACK);

  display.display();
  
  playTone(CORRECT_TONE, 1000);
}

void readSD() {
  Serial.begin(115200);
    Serial.println("Initializing");
    while (!Serial.available())
    {
        delay(10); // wait for serial port to connect. Needed for native USB
    }
    Serial.read(); // clear the initial input buffer
    Serial.println("Initialized");

    delay(1000);

    Serial.print("Initializing SD card...");

    if (!SD.begin(SD_CS_PIN))
    {
        Serial.println("initialization failed!");
        return;
    }
    Serial.println("initialization done.");

    // read SD

    myFile = SD.open(UNLOCK_CODE_FILE);
    if (myFile)
    {
        Serial.println(UNLOCK_CODE_FILE+":");

        // read from the file until there's nothing else in it:
        while (myFile.available())
        {
            Serial.write(myFile.read());
        }

        // close the file:
        myFile.close();
    }
    else
    {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }
    Serial.println("Done.");
}