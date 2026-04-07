/*
 * Gitarrentastatur
 * ESP32-S3-WROOM-1-N8R8
 * MCP23017-E/SO @ I2C Adresse 0x20
 * SK6812 MINI-E LEDs
 * USB-C (BLE MIDI)
 */

#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <FastLED.h>
#include <BLEMidi.h>

// ─── Konfiguration ───────────────────────────────────────────
#define NUM_LEDS        16      // Anzahl SK6812 LEDs
#define LED_DATA_PIN    38      // GPIO-Pin fuer LED-Datenleitung
#define LED_TYPE        SK6812
#define COLOR_ORDER     GRB

#define MCP_ADDR        0x20    // I2C Adresse des MCP23017
#define NUM_KEYS        16      // 16 Tasten (GPA0-7 + GPB0-7)

#define MIDI_CHANNEL    1       // MIDI Kanal (1-16)
#define BASE_NOTE       40      // Startton: E2 (tiefste Gitarrensaite offen)

// LED-Farben
#define COLOR_OFF       CRGB::Black
#define COLOR_PRESSED   CRGB::Red
#define COLOR_IDLE      CRGB(0, 5, 0)   // leichtes Gruen im Ruhezustand

// ─── Globale Variablen ────────────────────────────────────────
Adafruit_MCP23X17 mcp;
CRGB leds[NUM_LEDS];

uint16_t lastKeyState = 0xFFFF;  // HIGH = nicht gedrueckt (Pull-up)
bool bleConnected = false;

// ─── Setup ───────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println("Gitarrentastatur startet...");

  // I2C + MCP23017 initialisieren
  Wire.begin();
  if (!mcp.begin_I2C(MCP_ADDR)) {
    Serial.println("FEHLER: MCP23017 nicht gefunden!");
    while (1);
  }

  // Alle 16 Pins als Input mit Pull-up
  for (int i = 0; i < NUM_KEYS; i++) {
    mcp.pinMode(i, INPUT_PULLUP);
  }

  // SK6812 LEDs initialisieren
  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
         .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(50);
  ledsIdle();
  FastLED.show();

  // BLE MIDI initialisieren
  BLEMidiServer.begin("Gitarrentastatur");
  Serial.println("BLE MIDI bereit – warte auf Verbindung...");

  BLEMidiServer.setOnConnectCallback([]() {
    bleConnected = true;
    Serial.println("BLE verbunden!");
    ledsConnected();
    FastLED.show();
  });

  BLEMidiServer.setOnDisconnectCallback([]() {
    bleConnected = false;
    Serial.println("BLE getrennt.");
    ledsIdle();
    FastLED.show();
  });
}

// ─── Hauptschleife ────────────────────────────────────────────
void loop() {
  uint16_t currentKeyState = mcp.readGPIOAB();

  if (currentKeyState != lastKeyState) {
    for (int i = 0; i < NUM_KEYS; i++) {
      bool wasPressed = !(lastKeyState & (1 << i));
      bool isPressed  = !(currentKeyState & (1 << i));

      if (isPressed && !wasPressed) {
        // Taste gedrueckt
        uint8_t note = BASE_NOTE + i;
        Serial.printf("Taste %d gedrueckt -> Note %d\n", i, note);
        leds[i] = COLOR_PRESSED;
        if (bleConnected) {
          BLEMidiServer.noteOn(MIDI_CHANNEL, note, 100);
        }
      } else if (!isPressed && wasPressed) {
        // Taste losgelassen
        uint8_t note = BASE_NOTE + i;
        Serial.printf("Taste %d losgelassen -> Note %d\n", i, note);
        leds[i] = COLOR_IDLE;
        if (bleConnected) {
          BLEMidiServer.noteOff(MIDI_CHANNEL, note, 0);
        }
      }
    }
    FastLED.show();
    lastKeyState = currentKeyState;
  }

  delay(5);
}

// ─── LED-Hilfsfunktionen ─────────────────────────────────────
void ledsIdle() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = COLOR_IDLE;
  }
}

void ledsConnected() {
  // Kurzes Aufblinken in Blau bei BLE-Verbindung
  for (int i = 0; i < NUM_LEDS; i++) leds[i] = CRGB::Blue;
  FastLED.show();
  delay(300);
  ledsIdle();
}
