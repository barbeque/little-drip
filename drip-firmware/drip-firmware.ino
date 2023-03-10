// Pin definitions
// Selects for the 138s
#define SEL_0 P1_0 // KBD mode: column, LED mode: digit select
#define SEL_1 P1_1
#define SEL_2 P1_2
#define LED_DATA P1_3
#define LED_CLOCK P1_4
#define nLED_or_KBD_SEL P1_5

#define KBD_DATA P2_0
#define nKBD_ROW_SEL_A P2_1
#define nKBD_ROW_SEL_B P2_2
#define nKBD_ROW_SEL_C P2_3

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
  FONT_A = 0x1,
  FONT_B = 0x2,
  FONT_C = 0x4,
  FONT_D = 0x8,
  FONT_E = 0xF,
  FONT_F = 0x20,
  FONT_G = 0x40,
  FONT_DP = 0x80
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

/**
 * Blit a single character into a segment display
 */
void blitIntoSegmentDisplay(unsigned char character) {
  // TODO: is this backward? let's find out
  for(unsigned char i = 0; i < 8; ++i) {
    digitalWrite(LED_DATA, character & 0x01);
    digitalWrite(LED_CLOCK, HIGH);
    digitalWrite(LED_CLOCK, LOW);

    character = character >> 1;
  }
}

void refreshSegmentDisplay() {
  digitalWrite(nLED_or_KBD_SEL, LOW);

  /*
  for(unsigned int digit = 0; digit < 8; ++digit) {
    // from left to right
    // convert digit into binary and write to SEL_0, SEL_1, SEL_2
    // call blitIntoSegmentDisplay for this character
  }
  */

  /*
  for(unsigned int digit = 0; digit < 8; ++digit) {
    // leftmost digit
    digitalWrite(SEL_0, digit & 0x04);
    digitalWrite(SEL_1, digit & 0x02);
    digitalWrite(SEL_2, digit & 0x01);
    // rightmost digit
    blitIntoSegmentDisplay(digitTo7SegmentFont(digit));
  }
  */

  for(unsigned int x = 0; x <= 1; x++) {
    for(unsigned int y = 0; y <= 1; y++) {
      for(unsigned int z = 0; z <= 1; ++z) {
        // HACK
        digitalWrite(SEL_0, x);
        digitalWrite(SEL_1, y);
        digitalWrite(SEL_2, z);

        // should just light up the "A" segment of all 7 segs
        blitIntoSegmentDisplay(FONT_A);
      }
    }
  }
  
  digitalWrite(nLED_or_KBD_SEL, HIGH);
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

// TODO: Functions to get and set the value from current entry

void setup() {
  // put your setup code here, to run once:
  pinMode(KBD_DATA, INPUT);
  pinMode(DEC_LED, OUTPUT);
  pinMode(OCT_LED, OUTPUT);
  pinMode(HEX_LED, OUTPUT);
  pinMode(BIN_LED, OUTPUT);

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
  digitalWrite(nLED_or_KBD_SEL, 1);

  displayMode = DEC_LED;

  for(unsigned int i = 0; i < 8; ++i) {
    display[i] = 0;
  }
  insertionPoint = 0;
  
  updateModeLEDs();
}

void loop() {
  // put your main code here, to run repeatedly: 
  // Just poll for input here? I guess so
  // Do math ops as blocking?
  // Software debounce (delay?)

  // TODO: Figure out a timer interrupt
  refreshSegmentDisplay();
}
