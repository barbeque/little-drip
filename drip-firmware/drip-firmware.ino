// Pin definitions
// Selects for the 138s
#define SEL_0 P1_0 // KBD mode: column, LED mode: digit select
#define SEL_1 P1_1
#define SEL_2 P1_2
#define LED_DATA P1_3
#define LED_CLOCK P1_4
#define LED_LATCH P1_6
#define nLED_or_KBD_SEL P1_5

#define KBD_DATA P2_0
#define KBD_COL_SEL_A P2_1
#define KBD_COL_SEL_B P2_2
#define KBD_COL_SEL_C P2_3

enum LedMode {
  DEC_LED = P2_4,
  OCT_LED = P2_5,
  HEX_LED = P2_6,
  BIN_LED = P2_7
};

// TODO: Define LED 'font' and a way to clock it out
// TODO: Do a test for the letter "A" (LED A, B, C, F, E, G)

/*
 * 595 configuration:
 * QA - LED A (bit 0)
 * QB - LED B
 * QC - LED C
 * QD - LED D
 * QE - LED E
 * QF - LED F
 * QG - LED G
 * QH - LED D.P. (bit 7)
 */

enum FontSegment {
  FONT_A = 0x80,
  FONT_B = 0x40,
  FONT_C = 0x20,
  FONT_D = 0x10,
  FONT_E = 0x8,
  FONT_F = 0x4,
  FONT_G = 0x2,
  FONT_DP = 0x1
};

const unsigned char font[16] = {
  /* 0 => */ FONT_A | FONT_B | FONT_C | FONT_D | FONT_E | FONT_F,
  /* 1 => */ FONT_B | FONT_C,
  /* 2 => */ FONT_A | FONT_B | FONT_G | FONT_E | FONT_D,
  /* 3 => */ FONT_A | FONT_B | FONT_G | FONT_C | FONT_D,
  /* 4 => */ FONT_F | FONT_G | FONT_B | FONT_C,
  /* 5 => */ FONT_A | FONT_F | FONT_G | FONT_C | FONT_D,
  /* 6 => */ FONT_A | FONT_F | FONT_G | FONT_E | FONT_C | FONT_D,
  /* 7 => */ FONT_A | FONT_B | FONT_C,
  /* 8 => */ FONT_A | FONT_B | FONT_C | FONT_D | FONT_E | FONT_F | FONT_G,
  /* 9 => */ FONT_A | FONT_B | FONT_G | FONT_F | FONT_C | FONT_D,
  /* A => */ FONT_A | FONT_B | FONT_C | FONT_E | FONT_F | FONT_G,
  /* B => */ FONT_F | FONT_G | FONT_E | FONT_D | FONT_C,
  /* C => */ FONT_A | FONT_F | FONT_E | FONT_D,
  /* D => */ FONT_B | FONT_G | FONT_E | FONT_D | FONT_C,
  /* E => */ FONT_A | FONT_F | FONT_G | FONT_E | FONT_D,
  /* F => */ FONT_A | FONT_F | FONT_G | FONT_E
};

enum KeyCode {
  KEY_NIL = 0,
  KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
  KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F,
  KEY_BIN, KEY_DEC, KEY_HEX, KEY_OCT,
  KEY_CLEAR, KEY_STORE, KEY_RECALL, KEY_SHIFT,
  KEY_OR, KEY_AND, KEY_XOR,
  KEY_ADD, KEY_SUBTRACT, KEY_DIVIDE, KEY_MULTIPLY,
  KEY_K, KEY_1SCOMPLEMENT, KEY_DECIMAL, KEY_PLUSMINUS,
  KEY_CLEAR_ENTRY, KEY_ENTER
};

const KeyCode keyMap[8][5] = {
  { KEY_BIN, KEY_DEC, KEY_HEX, KEY_OCT, KEY_CLEAR },
  { KEY_STORE, KEY_RECALL, KEY_NIL, KEY_NIL, KEY_NIL },
  { KEY_SHIFT, KEY_D, KEY_E, KEY_F, KEY_K },
  { KEY_1SCOMPLEMENT, KEY_A, KEY_B, KEY_C, KEY_DIVIDE },
  { KEY_OR, KEY_7, KEY_8, KEY_9, KEY_MULTIPLY },
  { KEY_AND, KEY_4, KEY_5, KEY_6, KEY_SUBTRACT },
  { KEY_XOR, KEY_1, KEY_2, KEY_3, KEY_ADD },
  { KEY_CLEAR_ENTRY, KEY_0, KEY_DECIMAL, KEY_PLUSMINUS, KEY_ENTER }
};

KeyCode lastKeyPressed = KEY_NIL;

const KeyCode getKey() {
  // Must be in key-reading mode
  for(unsigned char row = 0; row < 8; ++row) {
    digitalWrite(SEL_0, row & 0x01);
    digitalWrite(SEL_1, row & 0x02);
    digitalWrite(SEL_2, row & 0x04);

    for(unsigned int col = 0; col < 5; ++col) {
      digitalWrite(KBD_COL_SEL_A, col & 0x01);
      digitalWrite(KBD_COL_SEL_B, col & 0x02);
      digitalWrite(KBD_COL_SEL_C, col & 0x04);

      // Now try to read - pretty sure this is active low
      if(!digitalRead(KBD_DATA)) {
        // TODO: Debounce?
        return keyMap[row][col];
      }
    }
  }

  return KEY_NIL; // nothing detected on this scan
}

// Note that 0 is the leftmost digit (MSB) and 7 is the rightmost (LSB)

// Calculator state
LedMode displayMode;
// TODO: Stack (how big can I make it?)
// TODO: Keyboard polling/interrupt?
// TODO: Screen refresh timer

// The current display, must be materialized into stack
// to do any operations
unsigned char display[8];
// Where the next key struck will be inserted into display buffer
unsigned char insertionPoint;

void turnOnLedDigit(unsigned char digit) {  
  // output 0: leftmost digit ($7)
  digitalWrite(SEL_0, digit & 0x01);
  digitalWrite(SEL_1, digit & 0x02);
  digitalWrite(SEL_2, digit & 0x04);
  // output 7: rightmost digit ($0)
}

/**
 * Blit a single character into a segment display
 */
void blitIntoSegmentDisplay(unsigned char character) {
  for(unsigned char i = 0; i < 8; ++i) {
    digitalWrite(LED_CLOCK, LOW);
    
    digitalWrite(LED_DATA, character & 0x01);
    
    digitalWrite(LED_CLOCK, HIGH);
    digitalWrite(LED_CLOCK, LOW);

    character = character >> 1;
  }

  // Cycle the latch so the 595 output will change
  digitalWrite(LED_LATCH, HIGH);
  digitalWrite(LED_LATCH, LOW);
}

/**
 * Update the "mode" LEDs to show what display
 * mode we are currently in (binary, hexadecimal, etc.)
 */
void updateModeLEDs() {
  digitalWrite(DEC_LED, displayMode == DEC_LED ? HIGH : LOW);
  digitalWrite(OCT_LED, displayMode == OCT_LED ? HIGH : LOW);
  digitalWrite(HEX_LED, displayMode == HEX_LED ? HIGH : LOW);
  digitalWrite(BIN_LED, displayMode == BIN_LED ? HIGH : LOW);
}

void changeDisplayMode(LedMode newMode) {
  displayMode = newMode;
  updateModeLEDs();
  // TODO: refresh display in new mode
}

void insertDigit(unsigned char digitValue) {
  display[insertionPoint] = digitValue;
  insertionPoint = max(7, insertionPoint + 1);
}

// TODO: Functions to get and set the value from current entry

void setup() {
  /*Serial.begin(9600);
  Serial.write("Hello");
  
  // put your setup code here, to run once:
  pinMode(KBD_DATA, INPUT);


  pinMode(SEL_0, OUTPUT);
  pinMode(SEL_1, OUTPUT);
  pinMode(SEL_2, OUTPUT);
  pinMode(nKBD_ROW_SEL_A, OUTPUT);
  pinMode(nKBD_ROW_SEL_B, OUTPUT);
  pinMode(nKBD_ROW_SEL_C, OUTPUT);

  pinMode(LED_DATA, OUTPUT);
  pinMode(LED_CLOCK, OUTPUT);
  pinMode(nLED_or_KBD_SEL, OUTPUT);
  // place into keyboard scan mode to prevent driving the display unnecessarily
  digitalWrite(nLED_or_KBD_SEL, 1);*/

  // Mode indicator LED selection
  pinMode(DEC_LED, OUTPUT);
  pinMode(OCT_LED, OUTPUT);
  pinMode(HEX_LED, OUTPUT);
  pinMode(BIN_LED, OUTPUT);

  changeDisplayMode(DEC_LED);

  pinMode(nLED_or_KBD_SEL, OUTPUT);
  pinMode(SEL_0, OUTPUT);
  pinMode(SEL_1, OUTPUT);
  pinMode(SEL_2, OUTPUT);
  pinMode(LED_DATA, OUTPUT);
  pinMode(LED_CLOCK, OUTPUT);
  pinMode(LED_LATCH, OUTPUT);

  pinMode(KBD_DATA, INPUT);
  pinMode(KBD_COL_SEL_A, OUTPUT);
  pinMode(KBD_COL_SEL_B, OUTPUT);
  pinMode(KBD_COL_SEL_C, OUTPUT);

  digitalWrite(LED_LATCH, LOW);

  for(unsigned int i = 0; i < 8; ++i) {
    display[i] = 0;
  }
  insertionPoint = 0;

  // I am a child
  display[0] = 0x0;
  display[1] = 0xb;
  display[2] = 0x0;
  display[3] = 0x0;
  display[4] = 0xb;
  display[5] = 0x1;
  display[6] = 0xe;
  display[7] = 0x5;
}

void loop() {  
  // LED time
  digitalWrite(nLED_or_KBD_SEL, LOW);

  for(unsigned int i = 0; i < 8; ++i) {
    blitIntoSegmentDisplay(font[display[i]]);
    turnOnLedDigit(i);
  }

  // Keyboard scanning time
  digitalWrite(nLED_or_KBD_SEL, HIGH);

  KeyCode justPressed = getKey();
  if(justPressed != KEY_NIL && justPressed != lastKeyPressed) {
    // TODO: Pass this into another handler. For now, we'll just do something stupid
    switch(justPressed) {
      case KEY_CLEAR:
        // Wipe out the display to prove input reading works
        for(unsigned int i = 0; i < 8; ++i) {
          display[i] = 0;
        }
        break;
      case KEY_DEC:
        changeDisplayMode(DEC_LED);
        break;
      case KEY_HEX:
        changeDisplayMode(HEX_LED);
        break;
      case KEY_BIN:
        changeDisplayMode(BIN_LED);
        break;
      case KEY_OCT:
        changeDisplayMode(OCT_LED);
        break;
      case KEY_F:
        insertDigit(0xf);
        break;
      case KEY_E:
        insertDigit(0xe);
        break;
      case KEY_D:
        insertDigit(0xd);
        break;
      case KEY_C:
        insertDigit(0xc);
        break;
      case KEY_B:
        insertDigit(0xb);
        break;
      case KEY_A:
        insertDigit(0xa);
        break;
      case KEY_9:
        insertDigit(0x9);
        break;
      case KEY_8:
        insertDigit(0x8);
        break;
      case KEY_7:
        insertDigit(0x7);
        break;
      case KEY_6:
        insertDigit(0x6);
        break;
      case KEY_5:
        insertDigit(0x5);
        break;
      case KEY_4:
        insertDigit(0x4);
        break;
      case KEY_3:
        insertDigit(0x3);
        break;
      case KEY_2:
        insertDigit(0x2);
        break;
      case KEY_1:
        insertDigit(0x1);
        break;
      case KEY_0:
        insertDigit(0x0);
        break;
    }
  }
}
