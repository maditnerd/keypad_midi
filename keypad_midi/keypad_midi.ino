#include "MIDIUSB.h"

/* Settings for MIDI preset */
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
const int NB_BUTTONS = ROWS * COLS;

const int PRESET = 8;
String preset_name[PRESET] = {"Du Riechst So Gut", "FF - Replica", "DrumKit", "A", "4", "5", "6", "B"};
int notes[PRESET][NB_BUTTONS] = {
  {38, 38, 39, 39, 43, 46, 38, 38, 48, 46, 45, 45, 57, 60, 60, 57}, //Preset 1
  {37, 40, 42, 39, 40, 38, 41, 40, 48, 46, 45, 45, 57, 60, 60, 57}, // Preset 2
  {35, 35, 38, 42, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 55, 57}, // Preset 3
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //Preset A
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //Preset 4
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //Preset 5
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},  //Preset 6
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}  //Preset B
};

int velocity[PRESET][NB_BUTTONS] = {
  {70, 70, 70, 70, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}, //Preset 1
  {70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70, 70}, //Preset 2
  {100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100}, //Preset 3
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //Preset A
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //Preset 4
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //Preset 5
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //Preset 6
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //Preset B
};

const int velocity_button = 15; // The D key is used to change velocity if velocity changer is true
const int VELOCITY_CHANGE = 100; //Velocity when D is pressed (should probably be changeable)
bool velocity_changer[PRESET] = {false, true, false, false, false, false, false, false};

/* Keypad Init */
//https://github.com/Nullkraft/Keypad
#include <Keypad.h>
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

//Keypad pins
byte rowPins[ROWS] = {8, 9, 10, 11}; //connect to the row pinouts of the kpd
byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the kpd
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//Keypad state : 1 = DOWN 3 = UP (there are other state which are unused here)
unsigned int buttons_state[NB_BUTTONS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
bool preset_selector = false;

//Oled Display
#include <Wire.h> //I2C
#include <U8g2lib.h>
U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
String oled_line1;
String oled_line2;

//Preset
int selected_preset = 0;

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  change_preset(selected_preset);
}

//Change preset and display it on screen
//You need to press "*" again to validate the preset
void change_preset(int preset) {
  if (preset < PRESET) {
    selected_preset = preset;
    oled_line1 = "-----Preset------";
    oled_line2 = preset_name[preset];
    printScreen();
    Serial.println(oled_line1);
    Serial.println(oled_line2);
  }
}

//Change what is written on the Oled screen using global variable
//oled_line1 and oled_line2
void printScreen() {
  //https://github.com/olikraus/u8g2/wiki/fntlist8x8
  u8g2.setFont(u8g2_font_7x14_mr);
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 14);
    u8g2.println(oled_line1);  // write something to the internal memory
    u8g2.setCursor(0, 30);
    u8g2.println(oled_line2);  // write something to the internal memory
  }
  while ( u8g2.nextPage() );
}

void loop() {
  if (kpd.getKeys()) {
    for (int i = 0; i < LIST_MAX; i++) // Scan the whole key list.
    {
      if ( kpd.key[i].stateChanged )   // Only find keys that have changed state.
      {
        buttons_state[kpd.key[i].kcode] = kpd.key[i].kstate;

        if (kpd.key[i].kstate == 1) { //DOWN
          Serial.print(kpd.key[i].kcode);
          Serial.print(":");
          Serial.println("1");

          //If key "#" is pressed
          if (kpd.key[i].kcode == 12) {
            //If preset selection is not active, we go to selection mode
            if (!preset_selector) {
              Serial.println("Selector");
              preset_selector = true;
              oled_line1 = "Selector";
              oled_line2 = "--------";
              printScreen();
            } else {
              //If preset selection is active, we return to midi mode
              Serial.println("Deselect");
              oled_line1 = preset_name[selected_preset];
              oled_line2 = "";
              printScreen();
              preset_selector = false;
            }
          } else {
            //If in selection mode we change preset
            if (preset_selector) {
              Serial.println("Change preset");
              change_preset(kpd.key[i].kcode);
            } else {
              //If in midi mode we play midi note
              key_down(kpd.key[i].kcode);
            }
          }
        }
        if (kpd.key[i].kstate == 3) { //UP
          Serial.print(kpd.key[i].kcode);
          Serial.print(":");
          Serial.println("0");
          // If key is up we send a midi note off
          key_up(kpd.key[i].kcode);
        }
      }
    }
  }
}

//Key down (pressed)
void key_down(int key) {
  int current_velocity = 100;
  //Serial.print(buttons_state[velocity_button]);

  //If velocity_changer (D) active
  if (velocity_changer[selected_preset]) {
    //If velocity changer pressed check for state 1 (down) or 2 (long down)
    if ((buttons_state[velocity_button] == 1) || (buttons_state[velocity_button] == 2)) {
      current_velocity = VELOCITY_CHANGE;
    } else {
      //If not pressed use velocity from preset
      current_velocity = velocity[selected_preset][key];
    }
  } else {
    //If velocity_changer not active, use velocity from preset
    current_velocity = velocity[selected_preset][key];
  }


  if (!velocity_changer[selected_preset]) {
    //Play note if velocity changer not set
    noteOn(0, notes[selected_preset][key], current_velocity);
  } else {
    //Else do not play note on velocity changer button (D)
    if (key != velocity_button) {
      noteOn(0, notes[selected_preset][key], current_velocity);
    }
  }

  //Display note and velocity on screen
  oled_line1 = preset_name[selected_preset];
  oled_line2 = "Note:" + String(notes[selected_preset][key]) + " " "V:" + String(current_velocity);
  printScreen();
  MidiUSB.flush();
}

//Key up (unpressed)
void key_up(int key) {
  //If velocity_changer
  if (!velocity_changer[selected_preset]) {
    noteOff(0, notes[selected_preset][key], 127);
  } else {
    if (key != velocity_button) {
      noteOff(0, notes[selected_preset][key], 127);
    }
  }
  MidiUSB.flush();
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
}
