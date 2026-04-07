# Gitarrentastatur

ESP32-basierter MIDI-Controller in Form einer Gitarrentastatur mit LED-Feedback.

## Bauteile

### ICs / Module
| Bezeichnung | Modell | Bemerkung |
|---|---|---|
| Mikrocontroller | ESP32-S3-WROOM-1-N8R8 | WiFi + BLE 5, nativer USB, 8MB Flash, 8MB PSRAM |
| I2C GPIO-Expander | MCP23017-E/SO | SOIC-28, I2C Adresse 0x20, RS Best.-Nr.: 403-816 |
| Spannungsregler | AMS1117 | 3,3V LDO, 5V → 3,3V |
| LEDs | SK6812 MINI-E | Adressierbare RGBW-LEDs (SMD) |

### Stecker / Buchsen
| Bezeichnung | Modell | Bemerkung |
|---|---|---|
| USB-C Buchse | TYPE-C-31-M-12 | USB-C, SMD |
| Programmierstecker | CON_Prog | 4-polig (TX, RX, 3V3, EN) |
| I2C Stecker | CON4 / J_I2C | 4-polig (SDA, SCK, 3V3, GND) |
| I2C Stecker 2 | J_I2C_2 | 4-polig |
| Erweiterungsstecker | CON3 | 3-polig |

### Taster / Schalter
| Bezeichnung | Anzahl | Bemerkung |
|---|---|---|
| T_EN Taster | 1 | Programmiermodus aktivieren |
| T_RST Taster | 1 | Reset |
| Rotary Encoder | 3 | Mit Druecktaster (encoder_1, encoder_2, encoder_3) |
| Gitarren-Schalter | mehrere | Matrix ROW/COL direkt am ESP32 (6 Reihen × n Spalten) |

### Widerstaende
| Bezeichnung | Wert | Funktion |
|---|---|---|
| R1 | 10 kΩ | T_EN Pull-up |
| R2 | 10 kΩ | I2C SCK Pull-up |
| R3 | 10 kΩ | I2C SDA Pull-up |
| R5 | 5,1 kΩ | USB-C CC1 |
| R6 | 5,1 kΩ | USB-C CC2 |
| R7 | 10 kΩ | T_RST Pull-up |
| R11, R12 | 10 kΩ | Encoder 1 (A, B) |
| R21, R22 | 10 kΩ | Encoder 2 (A, B) |
| R31, R32 | 10 kΩ | Encoder 3 (A, B) |
| R41 | 10 kΩ | MCP23017 RESET Pull-up |

### Kondensatoren
| Bezeichnung | Wert | Funktion |
|---|---|---|
| C1 | 10 µF | ESP32 Abblockkondensator |
| C2 | 1 µF | T_EN Entstoerung |
| C3, C4 | 10 µF | AMS1117 Ein-/Ausgangskondensatoren |
| C8 | 10 µF | MCP23017 Abblockkondensator |
| C11 – C40 | 30x 10 µF | Abblockkondensatoren (Versorgung) |

## Funktionsweise

- Gitarren-Tasten sind als **ROW/COL-Matrix** direkt am ESP32 angeschlossen (6 Saiten × mehrere Bunde)
- **MCP23017** (I2C, 0x20) liest die 3 **Rotary Encoder** ein (GPA/GPB)
- **SK6812 LEDs** geben visuelles Feedback pro Taste
- MIDI-Noten werden per **BLE MIDI** gesendet (z.B. an GarageBand, DAW)
- **AMS1117** wandelt die 5V (USB) auf 3,3V fuer ESP32 und MCP23017 um

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
