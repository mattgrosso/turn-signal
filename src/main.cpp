#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN 7
#define NUM_LEDS 60
#define NUM_SEATS 3

void resetLeds(CRGB flashColor);
void toggleSetupMode();
void setToggleState(int number, bool value);
void resetToggleStates();

unsigned long debounceDelay = 50;
const unsigned long longPressTime = 1000;
bool setupMode = false;

const int buttonPin1 = 2;
// bool button1State = false;

const int buttonPin2 = 3;
bool button2State = false;

const int buttonPin3 = 4;
bool button3State = false;

bool toggleStates[NUM_SEATS] = {false, false, false};

CRGB leds[NUM_LEDS];
CRGB* seat1 = &leds[0];
CRGB* seat2 = &leds[4];
CRGB* seat3 = &leds[8];
CRGB* seat4 = &leds[12];
CRGB colorGreen;
CRGB colorYellow;
CRGB colorRed;
CRGB colorBlack;
CRGB colorBlue;

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin1, INPUT);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(10);
  colorGreen = CRGB::Green;
  colorYellow = CRGB::Yellow;
  colorRed = CRGB::Red;
  colorBlack = CRGB::Black;
  colorBlue = CRGB::Blue;

  resetLeds(colorBlack);
  resetToggleStates();
}

void loop() {
  // Seat 1 button
  static unsigned long button1PressTime;
  static bool button1Pressed = false;
  static bool button1LongPressTriggered = false;
  int button1State = digitalRead(buttonPin1);

  if (button1State == HIGH) {
    if (!button1Pressed) {
      button1PressTime = millis();
      button1Pressed = true;
      button1LongPressTriggered = false;
    } else {
      unsigned long pressDuration = millis() - button1PressTime;
      if (pressDuration >= longPressTime && !button1LongPressTriggered) {
        // Button has been held down for longPressTime
        toggleSetupMode();
        button1LongPressTriggered = true;
      }
    }
  } else if (button1State == LOW && button1Pressed) {
    if (!button1LongPressTriggered) {
      // Short press
      if (*seat1 != colorGreen) {
        *seat1 = colorGreen;
        setToggleState(1, true);
      } else {
        *seat1 = colorBlack;
        setToggleState(1, false);
      }
      FastLED.show();
    }
    button1Pressed = false;
  }


  
  // Seat 2 button
  static unsigned long button2PressTime;
  static bool button2Pressed = false;
  static bool button2LongPressTriggered = false;
  int button2State = digitalRead(buttonPin2);

  if (button2State == HIGH) {
    if (!button2Pressed) {
      button2PressTime = millis();
      button2Pressed = true;
      button2LongPressTriggered = false;
    } else {
      unsigned long pressDuration = millis() - button2PressTime;
      if (pressDuration >= longPressTime && !button2LongPressTriggered) {
        // Button has been held down for longPressTime
        toggleSetupMode();
        button2LongPressTriggered = true;
      }
    }
  } else if (button2State == LOW && button2Pressed) {
    if (!button2LongPressTriggered) {
      // Short press
      if (*seat2 != colorGreen) {
        *seat2 = colorGreen;
        setToggleState(2, true);
      } else {
        *seat2 = colorBlack;
        setToggleState(2, false);
      }
      FastLED.show();
    }
    button2Pressed = false;
  }



  // Seat 3 button
  static unsigned long button3PressTime;
  static bool button3Pressed = false;
  static bool button3LongPressTriggered = false;
  int button3State = digitalRead(buttonPin3);

  if (button3State == HIGH) {
    if (!button3Pressed) {
      button3PressTime = millis();
      button3Pressed = true;
      button3LongPressTriggered = false;
    } else {
      unsigned long pressDuration = millis() - button3PressTime;
      if (pressDuration >= longPressTime && !button3LongPressTriggered) {
        // Button has been held down for longPressTime
        toggleSetupMode();
        button3LongPressTriggered = true;
      }
    }
  } else if (button3State == LOW && button3Pressed) {
    if (!button3LongPressTriggered) {
      // Short press
      if (*seat3 != colorGreen) {
        *seat3 = colorGreen;
        setToggleState(3, true);
      } else {
        *seat3 = colorBlack;
        setToggleState(3, false);
      }
      FastLED.show();
    }
    button3Pressed = false;
  }  
}

void resetLeds(CRGB flashColor) {
  for (int i = 0; i < 2; i++) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = flashColor;
    }

    FastLED.show();

    delay(250);

    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = colorBlack;
    }

    FastLED.show();

    delay(150);
  }

  delay(500);
}

void toggleSetupMode() {
  setupMode = !setupMode;

  if (setupMode) {
    resetToggleStates();
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = colorBlue;
    }
    FastLED.show();
  } else {
    resetLeds(colorGreen);
  }
}

void setToggleState(int number, bool value) {
  toggleStates[number - 1] = value;
}

void resetToggleStates() {
  for (int i = 0; i < NUM_SEATS; i++) {
    toggleStates[i] = false;
  }
}

void buttonHandler (int index) {
  static unsigned long buttonPressTime;
  static bool buttonPressed = false;
  static bool buttonLongPressTriggered = false;
  int buttonState = digitalRead(buttonPins[index]);

  if (buttonState == HIGH) {
    Serial.println("buttonState == HIGH");
    if (!buttonPressed) {
      Serial.println("!buttonPressed");
      buttonPressTime = millis();
      buttonPressed = true;
      buttonLongPressTriggered = false;
    } else {
      Serial.println("buttonPressed");
      unsigned long pressDuration = millis() - buttonPressTime;
      if (pressDuration >= longPressTime && !buttonLongPressTriggered) {
        // Button has been held down for longPressTime
        Serial.println("longPressTime");
        toggleSetupMode();
        buttonLongPressTriggered = true;
      }
    }
  } else if (buttonState == LOW && buttonPressed) {
    Serial.println("buttonState == LOW");
    if (!buttonLongPressTriggered) {
      Serial.println("!buttonLongPressTriggered");
      // Short press
      if (*seats[index] != colorGreen) {
        *seats[index] = colorGreen;
        setToggleState(index, true);
      } else {
        *seats[index] = colorBlack;
        setToggleState(index, false);
      }
      FastLED.show();
    }
    buttonPressed = false;
  }
}