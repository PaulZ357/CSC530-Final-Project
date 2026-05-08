#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SdFat.h>
#include <ESP_I2S.h>
#include <IRremote.h>
#include <Keyword_Detection_inferencing.h>   // <- rename to match your export
#include "RTClib.h"
#include <Arduino.h>

SdFat SD;
File32 myFile;
String fileName = "file_log.txt";
String UNLOCK_CODE_FILE = "unlock_log.txt";

// DMA chunk size for each I2S read in the capture task
#define CAPTURE_CHUNK   2048

static I2SClass    i2s;
static int16_t     capture_buf[CAPTURE_CHUNK];
const int CORRECT_TONE = 500;
const int INCORRECT_TONE = 1000;
const int speakerPin = D3;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// function declarations
static bool i2s_init();
static void audio_callback(uint32_t n_bytes);
static int  get_audio_signal_data(size_t offset, size_t length, float *out_ptr);
static bool inference_start(uint32_t n_samples);
static void inference_end();
static bool inference_record();
static void capture_task(void *arg);
static String runVoiceInference();
void readSD();
void showLockSymbol();
void showUnlockSymbol();
void playTone(int, int);
void log(String, String, String);
void resetUnlockFile();
void realTimeClock();
void incorrect();

static bool        record_status = false;

/* ── Inference buffer ──────────────────────────────────────────────────── */
typedef struct {
    int16_t  *buffer;
    uint8_t   buf_ready;
    uint32_t  buf_count;
    uint32_t  n_samples;
} inference_t;

/*void loop() {
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
            storedCode = myFile.read();
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

  display.clearDisplay(); // Clear buffer
  
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0,0);      // Start at top-left corner
  display.println(text);
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

#define SAMPLE_RATE     EI_CLASSIFIER_FREQUENCY          // typically 16000 Hz
#define SAMPLE_COUNT    EI_CLASSIFIER_RAW_SAMPLE_COUNT   // typically 16000 (1 s)
static bool        debug_nn      = false;
static inference_t inference;

String runVoiceInference() {
  // replace with Edge Impulse inference result
  int confidence = 0.90;

  // from testcontinuous
  signal_t signal;
  signal.total_length = SAMPLE_COUNT;
  signal.get_data     = &get_audio_signal_data;

  ei_impulse_result_t result = {};

  EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
  if (err != EI_IMPULSE_OK) {
      Serial.printf("[ERROR] run_classifier() returned %d\n", err);
      return "null";
  }

  /*Serial.printf("[INF] DSP: %d ms  | Inference: %d ms  | Anomaly: %d ms\n",
                result.timing.dsp,
                result.timing.classification,
                result.timing.anomaly);

  Serial.println("-- Scores --------------------------------------");
  for (uint16_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      Serial.printf("  %-20s %.4f\n",
                    result.classification[ix].label,
                    result.classification[ix].value);
  }*/

  #if EI_CLASSIFIER_HAS_ANOMALY == 1
      Serial.printf("  anomaly score       %.4f\n", result.anomaly);
  #endif

  float   best_val = 0.0f;
  uint8_t best_idx = 0;
  for (uint16_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
      if (result.classification[ix].value > best_val) {
          best_val = result.classification[ix].value;
          best_idx = ix;
      }
  }

  if (best_val >= confidence) {
    String label = result.classification[best_idx].label;
    Serial.printf(">>> DETECTED: %s (%.1f%%)\n",
      label,
      best_val * 100.0f);
    for (int i = 0; i < 7; i++) {
      String password = passwordWords[i];
      password.toLowerCase();
      label.toLowerCase();
      if (label == password) {
        Serial.println(label);
        return label;
      }
    }
  } else {
    Serial.printf(">>> No confident detection (best: %s @ %.1f%%)\n",
      result.classification[best_idx].label,
      best_val * 100.0f);
  }

  // example only:
  return "null";
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
  if (!i2s_init()) {
      Serial.println("[FATAL] PDM init failed — halting.");
      while (true) delay(1000);
  }

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK); // Start infrared decoding

    if (!inference_start(EI_CLASSIFIER_RAW_SAMPLE_COUNT)) {
        Serial.printf("[FATAL] Could not allocate audio buffer (size %d) — halting.\n",
                      EI_CLASSIFIER_RAW_SAMPLE_COUNT);
        while (true) delay(1000);
    }

  randomSeed(analogRead(0));

  pinMode(speakerPin, OUTPUT);
  readSD();
  resetUnlockFile();
  realTimeClock();
  
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
            
            log("CODE","PASS",enteredCode);

          } 
          else {
            codeCorrect = false;
            showOLED("ACCESS DENIED");
            delay(1000);
            currentState = LOCKED;
                
            // buzz for incorrect
            showLockSymbol();
            incorrect();

            log("CODE","FAIL",enteredCode);

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

      String recognizedWord = runVoiceInference();

      recognizedWord.toLowerCase();

      while (recognizedWord == "null") {
        recognizedWord = runVoiceInference();
        recognizedWord.toLowerCase();
      }

      if (recognizedWord == expectedWord) {
        voiceCorrect = true;
      } 
      else {
        voiceCorrect = false;
      }

      if (codeCorrect && voiceCorrect) {
        currentState = UNLOCKED;

        // buzz for correct
        showUnlockSymbol();

        // log 
        log("VOICE","PASS",recognizedWord);
      } 
      else {
        showOLED("ACCESS DENIED. "+recognizedWord+" x "+expectedWord);

        delay(1000);
        currentState = LOCKED;
        
        // buzz for incorrect
        showLockSymbol();
        incorrect();

        // log 
        log("VOICE","FAIL",recognizedWord);
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

const char* formattedString = "%02d:%02d:%02d:%02d:%02d:%02d,%d\n";
int elapsed;
DateTime previous;
RTC_PCF8563 rtc;

void log(String stage, String result, String detail) {
  const DateTime current = rtc.now();

  const int year = current.year() % 100;
  const int month = current.month();
  const int day = current.day();
  const int hour = current.hour();
  const int minute = current.minute();
  const int second = current.second();
  const int total = current.unixtime() - previous.unixtime();

  elapsed = total;

  myFile = SD.open(UNLOCK_CODE_FILE, FILE_WRITE);
  if (myFile) {
    myFile.printf(formattedString, year, month, day, hour, minute, second, stage, result, detail);
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening "+UNLOCK_CODE_FILE);
  }
}

void resetUnlockFile() {
  
  myFile = SD.open(UNLOCK_CODE_FILE, FILE_WRITE);
  myFile.seek(0); // Move to the beginning of the file
  myFile.truncate(); // Remove all contents
  myFile.close();

}

// helper functions from Test Continuous
static int get_audio_signal_data(size_t offset, size_t length, float *out_ptr)
{
    numpy::int16_to_float(inference.buffer + offset, out_ptr, length);
    return EIDSP_OK;
}

/* ── Allocate inference buffer and launch the capture task ─────────────── */
static bool inference_start(uint32_t n_samples)
{
    inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));
    if (!inference.buffer) return false;

    inference.buf_count = 0;
    inference.n_samples = n_samples;
    inference.buf_ready = 0;

    ei_sleep(100);
    record_status = true;

    xTaskCreate(capture_task, "CaptureTask", 1024 * 32, (void *)CAPTURE_CHUNK, 10, NULL);

    return true;
}
/* ── Block until a full inference window is ready ──────────────────────── */
static bool inference_record()
{
    while (inference.buf_ready == 0) delay(10);
    inference.buf_ready = 0;
    return true;
}

/* ── Stop the capture task and free the inference buffer ───────────────── */
static void inference_end()
{
    record_status = false;
    ei_free(inference.buffer);
}

/* ── FreeRTOS task: continuously reads I2S and feeds the inference buffer ─ */
static void capture_task(void *arg)
{
    const int32_t bytes_to_read = (uint32_t)arg;

    while (record_status) {
        size_t got = i2s.readBytes((char *)capture_buf, bytes_to_read);

        if (got <= 0) {
            Serial.printf("[ERROR] i2s.readBytes() returned %d\n", got);
        } else {
            if (got < (size_t)bytes_to_read)
                Serial.println("[WARN] Partial I2S read");

            // Scale up to compensate for the mic's low output level
            for (int x = 0; x < bytes_to_read / 2; x++)
                capture_buf[x] = (int16_t)(capture_buf[x]) * 8;

            if (record_status)
                audio_callback(bytes_to_read);
        }
    }

    vTaskDelete(NULL);
}

/* ── Fill inference buffer one DMA chunk at a time ─────────────────────── */
static void audio_callback(uint32_t n_bytes)
{
    for (uint32_t i = 0; i < n_bytes >> 1; i++) {
        inference.buffer[inference.buf_count++] = capture_buf[i];

        if (inference.buf_count >= inference.n_samples) {
            inference.buf_count = 0;
            inference.buf_ready = 1;
        }
    }
}


/* ── PDM initialisation ────────────────────────────────────────────────── */
#define PDM_CLK_PIN     42   // GPIO 42 → MSM261D3526H1CPM CLK
#define PDM_DATA_PIN    41   // GPIO 41 → MSM261D3526H1CPM DATA
#define WARMUP_MS       30
static bool i2s_init()
{
    // Dedicated PDM RX pin setter — no BCLK, WS, or MCLK needed for PDM
    i2s.setPinsPdmRx(PDM_CLK_PIN, PDM_DATA_PIN);

    // I2S_MODE_PDM_RX  : PDM receive path with hardware decimation to PCM
    // 16-bit width     : only supported width for PDM RX
    // MONO             : single onboard mic; L/R is fixed to GND on the PCB
    bool ok = i2s.begin(I2S_MODE_PDM_RX,
                        SAMPLE_RATE,
                        I2S_DATA_BIT_WIDTH_16BIT,
                        I2S_SLOT_MODE_MONO);

    if (!ok) {
        Serial.println("[ERROR] i2s.begin(I2S_MODE_PDM_RX) failed.");
        return false;
    }

    // Discard the first WARMUP_MS of samples to clear the MSM261's power-up
    // transient before any inference recording starts.
    const size_t warmup_samples = (size_t)(SAMPLE_RATE * WARMUP_MS / 1000);
    char *discard = (char *)malloc(warmup_samples * sizeof(int16_t));
    if (discard) {
        i2s.readBytes(discard, warmup_samples * sizeof(int16_t));
        free(discard);
    }

    return true;
}

 void realTimeClock() {
  
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    rtc.adjust(DateTime(2026, 3, 8, 0, 7, 0));
  }

  rtc.start();

 }