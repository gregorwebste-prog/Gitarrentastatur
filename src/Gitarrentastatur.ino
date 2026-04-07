/*
 * Gitarrentastatur
 * ESP32-S3-WROOM-1-N8R8
 *
 * Matrix:    6 Reihen (Saiten) x 14 Spalten (Buende) = 84 Tasten
 * LEDs:      94x SK6812 in Serie (10 Logo + 84 Tasten)
 * Encoder:   3x Rotary Encoder via MCP23017 (I2C 0x20)
 * MIDI:      BLE MIDI
 *
 * WICHTIG: GPIO-Nummern unten gegen deine Schematic pruefen!
 */

#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <FastLED.h>
#include <BLEMidi.h>

// ═══════════════════════════════════════════════════════════════
//  KONFIGURATION – hier anpassen
// ═══════════════════════════════════════════════════════════════

// Matrix-Groesse
#define NUM_ROWS    6
#define NUM_COLS    14

// GPIO-Pins fuer die Matrix (bitte gegen Schematic pruefen!)
const uint8_t ROW_PINS[NUM_ROWS] = {36, 35, 34, 33, 26, 21};  // ROW_1 bis ROW_6
const uint8_t COL_PINS[NUM_COLS] = {20, 19, 18, 17, 16, 15, 7, 6, 5, 4, 3, 2, 1, 0};  // COL_0 bis COL_13

// LEDs
#define LED_PIN         38      // DIN-Pin fuer SK6812 (aus Schematic: DIN LED)
#define NUM_LEDS        94      // Gesamt (10 Logo + 84 Tasten)
#define LOGO_LEDS       10      // Erste 10 LEDs = Logo
#define KEY_LED_OFFSET  10      // Ab Index 10 beginnen Tasten-LEDs
#define LED_BRIGHTNESS  60

// MCP23017 (Encoder)
#define MCP_ADDR    0x20
// Encoder-Pins auf MCP23017 (GPA = Port A, GPB = Port B)
// ENC_1: GPA0 (A), GPA1 (B), GPB0 (Taster)
// ENC_2: GPA2 (A), GPA3 (B), GPB1 (Taster)
// ENC_3: GPA4 (A), GPA5 (B), GPB2 (Taster)
#define ENC1_A   0   // MCP Pin GPA0
#define ENC1_B   1   // MCP Pin GPA1
#define ENC2_A   2   // MCP Pin GPA2
#define ENC2_B   3   // MCP Pin GPA3
#define ENC3_A   4   // MCP Pin GPA4
#define ENC3_B   5   // MCP Pin GPA5
#define ENC1_SW  8   // MCP Pin GPB0
#define ENC2_SW  9   // MCP Pin GPB1
#define ENC3_SW  10  // MCP Pin GPB2

// MIDI
#define MIDI_CHANNEL    1
#define MIDI_VELOCITY   100

// Gitarren-Stimmung (E-Standard): MIDI-Noten der offenen Saiten
// Reihe 0 = Saite 1 (hohe e), Reihe 5 = Saite 6 (tiefe E)
const uint8_t OPEN_STRING_NOTES[NUM_ROWS] = {
  64,  // Saite 1: e4
  59,  // Saite 2: B3
  55,  // Saite 3: G3
  50,  // Saite 4: D3
  45,  // Saite 5: A2
  40   // Saite 6: E2
};

// LED-Farben
#define COLOR_LOGO      CRGB(0, 80, 255)   // Blau fuer Logo
#define COLOR_IDLE      CRGB(0, 3, 0)      // Minimales Gruen im Ruhezustand
#define COLOR_PRESSED   CRGB(255, 0, 0)    // Rot wenn gedrueckt
#define COLOR_CONNECTED CRGB(0, 0, 120)    // Blau bei BLE-Verbindung

// ═══════════════════════════════════════════════════════════════
//  GLOBALE VARIABLEN
// ═══════════════════════════════════════════════════════════════

Adafruit_MCP23X17 mcp;
CRGB leds[NUM_LEDS];

// Tastenstatus: true = gedrueckt
bool keyState[NUM_ROWS][NUM_COLS];
bool lastKeyState[NUM_ROWS][NUM_COLS];

// Encoder-Zustand
uint8_t lastEncState[3];
int encValue[3] = {0, 0, 0};

bool bleConnected = false;

// ═══════════════════════════════════════════════════════════════
//  HILFSFUNKTIONEN
// ═══════════════════════════════════════════════════════════════

// Gibt den LED-Index fuer eine Taste zurueck
int keyToLED(int row, int col) {
  return KEY_LED_OFFSET + (row * NUM_COLS) + col;
}

// Gibt die MIDI-Note fuer eine Taste zurueck
uint8_t keyToNote(int row, int col) {
  return OPEN_STRING_NOTES[row] + col;  // Jeder Bund = 1 Halbton hoeher
}

void setLogoLEDs(CRGB color) {
  for (int i = 0; i < LOGO_LEDS; i++) {
    leds[i] = color;
  }
}

void setAllKeyLEDs(CRGB color) {
  for (int i = KEY_LED_OFFSET; i < NUM_LEDS; i++) {
    leds[i] = color;
  }
}

void animateStartup() {
  // Logo auffuellen
  for (int i = 0; i < LOGO_LEDS; i++) {
    leds[i] = COLOR_LOGO;
    FastLED.show();
    delay(40);
  }
  // Tasten nacheinander aufleuchten
  for (int i = KEY_LED_OFFSET; i < NUM_LEDS; i++) {
    leds[i] = COLOR_IDLE;
    FastLED.show();
    delay(5);
  }
}

// ═══════════════════════════════════════════════════════════════
//  MATRIX-SCAN
// ═══════════════════════════════════════════════════════════════

void scanMatrix() {
  for (int row = 0; row < NUM_ROWS; row++) {
    // Aktuelle Reihe LOW schalten
    digitalWrite(ROW_PINS[row], LOW);

    delayMicroseconds(10);  // kurz warten bis Signal stabil

    for (int col = 0; col < NUM_COLS; col++) {
      keyState[row][col] = (digitalRead(COL_PINS[col]) == LOW);
    }

    // Reihe wieder HIGH
    digitalWrite(ROW_PINS[row], HIGH);
  }
}

void processMatrix() {
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < NUM_COLS; col++) {
      bool pressed = keyState[row][col];
      bool wasPressed = lastKeyState[row][col];

      if (pressed && !wasPressed) {
        // Taste gedrueckt
        uint8_t note = keyToNote(row, col);
        int ledIdx = keyToLED(row, col);

        leds[ledIdx] = COLOR_PRESSED;

        Serial.printf("Gedrueckt: Saite %d, Bund %d -> Note %d\n", row + 1, col, note);

        if (bleConnected) {
          BLEMidiServer.noteOn(MIDI_CHANNEL, note, MIDI_VELOCITY);
        }
      } else if (!pressed && wasPressed) {
        // Taste losgelassen
        uint8_t note = keyToNote(row, col);
        int ledIdx = keyToLED(row, col);

        leds[ledIdx] = COLOR_IDLE;

        if (bleConnected) {
          BLEMidiServer.noteOff(MIDI_CHANNEL, note, 0);
        }
      }

      lastKeyState[row][col] = pressed;
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  ENCODER-SCAN (via MCP23017)
// ═══════════════════════════════════════════════════════════════

// Encoder-Pins auf MCP (A, B)
const uint8_t ENC_A[3] = {ENC1_A, ENC2_A, ENC3_A};
const uint8_t ENC_B[3] = {ENC1_B, ENC2_B, ENC3_B};
const uint8_t ENC_SW[3] = {ENC1_SW, ENC2_SW, ENC3_SW};

// MIDI CC-Nummern fuer die Encoder (z.B. Volume, Pan, Expression)
const uint8_t ENC_CC[3] = {7, 10, 11};

void readEncoders() {
  for (int e = 0; e < 3; e++) {
    uint8_t a = mcp.digitalRead(ENC_A[e]);
    uint8_t b = mcp.digitalRead(ENC_B[e]);
    uint8_t state = (a << 1) | b;

    if (state != lastEncState[e]) {
      // Drehrichtung bestimmen
      if ((lastEncState[e] == 0b10 && state == 0b00) ||
          (lastEncState[e] == 0b01 && state == 0b11)) {
        encValue[e] = constrain(encValue[e] + 5, 0, 127);
        Serial.printf("Encoder %d: +1 -> %d\n", e + 1, encValue[e]);
        if (bleConnected) {
          BLEMidiServer.controlChange(MIDI_CHANNEL, ENC_CC[e], encValue[e]);
        }
      } else if ((lastEncState[e] == 0b00 && state == 0b10) ||
                 (lastEncState[e] == 0b11 && state == 0b01)) {
        encValue[e] = constrain(encValue[e] - 5, 0, 127);
        Serial.printf("Encoder %d: -1 -> %d\n", e + 1, encValue[e]);
        if (bleConnected) {
          BLEMidiServer.controlChange(MIDI_CHANNEL, ENC_CC[e], encValue[e]);
        }
      }
      lastEncState[e] = state;
    }

    // Encoder-Taster
    if (!mcp.digitalRead(ENC_SW[e])) {
      Serial.printf("Encoder %d Taster gedrueckt\n", e + 1);
      // z.B. Wert zuruecksetzen
      encValue[e] = 64;
      if (bleConnected) {
        BLEMidiServer.controlChange(MIDI_CHANNEL, ENC_CC[e], 64);
      }
      delay(200);  // Entprellen
    }
  }
}

// ═══════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  Serial.println("Gitarrentastatur startet...");

  // Matrix: ROW = OUTPUT HIGH, COL = INPUT_PULLUP
  for (int r = 0; r < NUM_ROWS; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], HIGH);
  }
  for (int c = 0; c < NUM_COLS; c++) {
    pinMode(COL_PINS[c], INPUT_PULLUP);
  }

  // Tastenstatus zuruecksetzen
  memset(keyState, 0, sizeof(keyState));
  memset(lastKeyState, 0, sizeof(lastKeyState));

  // MCP23017 initialisieren
  Wire.begin();
  if (!mcp.begin_I2C(MCP_ADDR)) {
    Serial.println("FEHLER: MCP23017 nicht gefunden!");
    while (1);
  }
  // Encoder-Pins als Input mit Pull-up
  for (int e = 0; e < 3; e++) {
    mcp.pinMode(ENC_A[e], INPUT_PULLUP);
    mcp.pinMode(ENC_B[e], INPUT_PULLUP);
    mcp.pinMode(ENC_SW[e], INPUT_PULLUP);
    lastEncState[e] = (mcp.digitalRead(ENC_A[e]) << 1) | mcp.digitalRead(ENC_B[e]);
  }

  // SK6812 LEDs initialisieren
  FastLED.addLeds<SK6812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(LED_BRIGHTNESS);
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  animateStartup();

  // BLE MIDI
  BLEMidiServer.begin("Gitarrentastatur");
  Serial.println("BLE MIDI bereit – warte auf Verbindung...");

  BLEMidiServer.setOnConnectCallback([]() {
    bleConnected = true;
    Serial.println("BLE verbunden!");
    setLogoLEDs(COLOR_CONNECTED);
    FastLED.show();
  });

  BLEMidiServer.setOnDisconnectCallback([]() {
    bleConnected = false;
    Serial.println("BLE getrennt.");
    setLogoLEDs(COLOR_LOGO);
    FastLED.show();
  });
}

// ═══════════════════════════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════════════════════════

void loop() {
  scanMatrix();
  processMatrix();
  readEncoders();

  FastLED.show();
  delay(5);
}
