# Gitarrentastatur

ESP32-basierter MIDI-Controller in Form einer Gitarrentastatur mit LED-Feedback.

## Hardware

| Bauteil | Modell | Bemerkung |
|---|---|---|
| Mikrocontroller | ESP32-S3-WROOM-1-N8R8 | WiFi + BLE, nativer USB |
| I2C GPIO-Expander | MCP23017-E/SO | Adresse: 0x20, RS Best.-Nr.: 403-816 |
| LEDs | SK6812 MINI-E | Adressierbare RGB-LEDs |
| USB-Anschluss | TYPE-C-31-M-12 | USB-C |

## Funktionsweise

- 16 Tasten werden ueber den **MCP23017** (I2C, 0x20) eingelesen
- Jede Taste steuert eine **SK6812 LED** an (visuelles Feedback)
- MIDI-Noten werden per **BLE MIDI** gesendet (z.B. an GarageBand, DAW)
- Startton: **E2 (MIDI Note 40)** – tiefste offene Gitarrensaite

## Benoettigte Libraries (Arduino IDE)

1. `Adafruit MCP23017` – I2C GPIO-Expander
2. `FastLED` – SK6812 LEDs
3. `BLE-MIDI` von Lathoub – BLE MIDI Server

Installieren ueber: **Sketch → Bibliotheken verwalten**

## ESP32-S3 Programmieren

1. **T_RST** gedrueckt halten
2. Zusaetzlich **T_EN** druecken
3. Beide wieder loslassen → Geraet ist jetzt im Programmiermodus
4. In Arduino IDE: Upload starten

> **Wichtig:** Upload erst moeglich nach dieser Tastenkombination!

### Serieller Monitor

- Fuer den seriellen Monitor wird die Extension **"ESP32 USB CDC"** benoetigt
- Vor dem erneuten Programmieren muss der serielle Monitor **gestoppt** werden

## Board-Einstellungen (Arduino IDE)

| Einstellung | Wert |
|---|---|
| Board | ESP32S3 Dev Module |
| USB Mode | USB-OTG (TinyUSB) |
| Upload Mode | UART0 / Hardware CDC |
| Flash Size | 8MB |
| PSRAM | OPI PSRAM |

## Zeitprotokoll

| Datum | Zeit | Taetigkeit |
|---|---|---|
| Vor 17.2.2026 | ~7h | Grundkonzept, Libraries, Schematic |
| Vor 17.2.2026 | ~1h | Waehrend Religion |
| 17.2.2026 | 1h | |
| 18.2.2026 | 3h | |
| 22.2.2026 | 2,5h | |
| 1.3.2026 | 1h | Bauteile suchen, letzte Aenderungen |
| 21.3.2026 | 2h | Neuer Switch-Footprint, neue Traces |
| 25.3.2026 | 3h | Bauteile ausgesucht, bestellt, Platine bestueckt, programmiert |
| 26.3.2026 | 20min | Taster und Dioden loeten |
| 27.3.2026 | 30min | Testen |
| **Gesamt** | **~21h** | |
