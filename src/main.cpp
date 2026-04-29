#include <Arduino.h>
#include <SdFat.h>

SdFat SD;
File32 myFile;
String fileName = "file_log.txt";
String UNLOCK_CODE_FILE = "unlock.txt";

void setup()
{
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

void loop()
{
    // put your main code here, to run repeatedly:
    Serial.println("test");
    delay(1000);
}
