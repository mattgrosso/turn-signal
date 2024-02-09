#include <Arduino.h>
#include <FastLED.h>

// LED settings
#define LED_PIN 7
#define NUM_LEDS 60
#define NUM_SEATS 3
CRGB leds[NUM_LEDS];
CRGB* seats[NUM_SEATS] = {&leds[0], &leds[4], &leds[8]};
CRGB colorGreen;
CRGB colorYellow;
CRGB colorRed;
CRGB colorBlack;
CRGB colorBlue;

// Button settings
const int buttonPins[NUM_SEATS] = {2, 3, 4};
#define LONG_PRESS_TIME 1000
static unsigned long buttonPressTime[NUM_SEATS];
static bool buttonPressed[NUM_SEATS] = {false};
static bool buttonLongPressTriggered[NUM_SEATS] = {false};

// State variables
bool toggleStates[NUM_SEATS] = {false, false, false};
bool setupMode = false;

// Function prototypes
void resetLeds(CRGB flashColor);
void toggleSetupMode();
void setToggleState(int number, bool value);
void resetToggleStates();
void buttonHandler(int index);

void setup() {
  Serial.begin(9600);

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
  buttonHandler(0);
  buttonHandler(1);
  buttonHandler(2);
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
  int buttonState = digitalRead(buttonPins[index]);

  if (buttonState == HIGH) {
    if (!buttonPressed[index]) {
      buttonPressTime[index] = millis();
      buttonPressed[index] = true;
      buttonLongPressTriggered[index] = false;
    } else {
      unsigned long pressDuration = millis() - buttonPressTime[index];
      if (pressDuration >= LONG_PRESS_TIME && !buttonLongPressTriggered[index]) {
        // Button has been held down for LONG_PRESS_TIME
        toggleSetupMode();
        buttonLongPressTriggered[index] = true;
      }
    }
  } else if (buttonState == LOW && buttonPressed[index]) {
    Serial.println("buttonState == LOW");
    if (!buttonLongPressTriggered[index]) {
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
    buttonPressed[index] = false;
  }
}