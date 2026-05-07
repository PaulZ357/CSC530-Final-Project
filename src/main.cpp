#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SdFat.h>
#include <IRremote.h>
#include <Keyword_Detection_inferencing.h>   // <- rename to match your export

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

/*void setup() {
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
}*/

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

#include <Arduino.h>

enum State {
  LOCKED,
  CODE_ENTRY,
  VOICE_ENTRY,
  UNLOCKED
};

State currentState = LOCKED;

String storedCode = "1234";   // Later read from SD card
String enteredCode = "";

bool codeCorrect = false;
bool voiceCorrect = false;

const char* passwordWords[] = {
  "blue", "cyan", "green", "magenta", "red", "white", "yellow"
};

int passwordIndex = 0;
String expectedWord = "";

// ---------- PLACEHOLDER FUNCTIONS ----------

void showOLED(String text) {
  Serial.println(text);
  // replace with OLED display code
}

char readIRDigit() {
  // replace with your IR remote code
  // return '0' to '9' when pressed
  // return '\0' if nothing pressed
  if (IrReceiver.decode()) {
      uint32_t code = IrReceiver.decodedIRData.decodedRawData;
      Serial.print("Code: ");
      Serial.println(code, HEX);
      // Example mapping for common remotes
      int pressedDigit;
      switch(code) {
          case 0xE916FF00:
              Serial.println("0 pressed");
              pressedDigit = '0';
              break;
          case 0xF30CFF00:
              Serial.println("1 pressed");
              pressedDigit = '1';
              break;
          case 0xE718FF00:
              Serial.println("2 pressed");
              pressedDigit = '2';
              break;
          case 0xA15EFF00:
              Serial.println("3 pressed");
              pressedDigit = '3';
              break;
          case 0xF708FF00:
              Serial.println("4 pressed");
              pressedDigit = '4';
              break;
          case 0xE31CFF00:
              Serial.println("5 pressed");
              pressedDigit = '5';
              break;
          case 0xA55AFF00:
              Serial.println("6 pressed");
              pressedDigit = '6';
              break;
          case 0xBD42FF00:
              Serial.println("7 pressed");
              pressedDigit = '7';
              break;
          case 0xAD52FF00:
              Serial.println("8 pressed");
              pressedDigit = '8';
              break;
          case 0xB54AFF00:
              Serial.println("9 pressed");
              pressedDigit = '9';
              break;
          default:
              pressedDigit = '\0';
      }
      IrReceiver.resume();
      return pressedDigit;
  }
  return '\0';
}

String runVoiceInference(float &confidence) {
  // replace with Edge Impulse inference result
  confidence = 0.90;

  // example only:
  return "blue";
}

// ---------- MAIN LOGIC ----------

const byte IR_RECEIVE_PIN = A0;

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing");
  while (!Serial.available())
  {
      delay(10); // wait for serial port to connect. Needed for native USB
  }
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start infrared decoding

  randomSeed(analogRead(0));

  currentState = LOCKED;
  showOLED("LOCK");
}

void loop() {
  switch (currentState) {

    case LOCKED:
      codeCorrect = false;
      voiceCorrect = false;
      enteredCode = "";

      showOLED("LOCK");
      delay(500);

      currentState = CODE_ENTRY;
      break;

    case CODE_ENTRY: {
      showOLED("Enter Code");

      char digit = readIRDigit();
      Serial.println(digit);

      if (digit >= '0' && digit <= '9') {
        enteredCode += digit;

        showOLED("Code: " + enteredCode);

        if (enteredCode.length() == 4) {
          if (enteredCode == storedCode) {
            codeCorrect = true;

            passwordIndex = random(0, 7);
            expectedWord = passwordWords[passwordIndex];

            showOLED("Speak #" + String(passwordIndex + 1));

            currentState = VOICE_ENTRY;
          } 
          else {
            codeCorrect = false;
            showOLED("ACCESS DENIED");
            delay(1000);
            currentState = LOCKED;
          }
        }
      }

      break;
    }

    case VOICE_ENTRY: {
      if (codeCorrect == false) {
        currentState = LOCKED;
        break;
      }

      showOLED("Speak now");

      float confidence = 0.0;
      String recognizedWord = runVoiceInference(confidence);

      recognizedWord.toLowerCase();

      if (recognizedWord == expectedWord && confidence >= 0.80) {
        voiceCorrect = true;
      } 
      else {
        voiceCorrect = false;
      }

      if (codeCorrect && voiceCorrect) {
        currentState = UNLOCKED;
      } 
      else {
        showOLED("ACCESS DENIED");
        delay(1000);
        currentState = LOCKED;
      }

      break;
    }

    case UNLOCKED:
      showOLED("UNLOCKED");

      // buzzer success here if needed

      delay(3000);

      currentState = LOCKED;
      break;
  }
  delay(100); // avoid too fast refresh
}