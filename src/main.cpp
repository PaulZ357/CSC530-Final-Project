#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SdFat.h>

SdFat SD;
File32 myFile;
String fileName = "file_log.txt";
String UNLOCK_CODE_FILE = "unlock.txt";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// function declarations
void readSD();

void setup() {
  readSD();
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

void loop() {
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