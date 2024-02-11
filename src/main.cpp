#include <Arduino.h>
#include <FastLED.h>

// LED settings
#define LED_PIN 7
#define NUM_LEDS 60
#define NUM_SEATS 3
#define seat1 0
#define seat2 10
#define seat3 20
int seatIndices[NUM_SEATS] = {seat1, seat2, seat3};
CRGB leds[NUM_LEDS];
CRGB* seatLEDs[NUM_SEATS] = {&leds[seat1], &leds[seat2], &leds[seat3]};
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
bool occupiedSeats[NUM_SEATS] = {false, false, false};
bool setupMode = false;
int currentPlayer;

// Function prototypes
void resetLeds(CRGB flashColor);
void toggleSetupMode();
void setOccupiedSeat(int number, bool value);
void resetOccupiedSeats();
void buttonHandler(int index);
void chooseStartPlayer();
void goToNextPlayer ();

void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(30);
  colorGreen = CRGB::Green;
  colorYellow = CRGB::Yellow;
  colorRed = CRGB::Red;
  colorBlack = CRGB::Black;
  colorBlue = CRGB::Blue;

  resetLeds(colorBlack);
  resetOccupiedSeats();
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
    resetOccupiedSeats();
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = colorBlue;
    }
    FastLED.show();
  } else {
    resetLeds(colorGreen);
    chooseStartPlayer();
  }
}

void setOccupiedSeat(int index, bool value) {
  occupiedSeats[index] = value;
}

void resetOccupiedSeats() {
  for (int i = 0; i < NUM_SEATS; i++) {
    occupiedSeats[i] = false;
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
        Serial.println("Long press detected");
        toggleSetupMode();
        buttonLongPressTriggered[index] = true;
      }
    }
  } else if (buttonState == LOW && buttonPressed[index]) {
    if (!buttonLongPressTriggered[index]) {
      Serial.println("Short press detected");
      if (setupMode) {
        if (!occupiedSeats[index]) {
          setOccupiedSeat(index, true);
          *seatLEDs[index] = colorGreen;
          FastLED.show();
        } else {
          setOccupiedSeat(index, false);
          *seatLEDs[index] = colorBlue;
          FastLED.show();
        }
      } else {
        // Normal mode
        if (index == currentPlayer) {
          goToNextPlayer();
        } else {
          Serial.println("It's not your turn!");
        }
      }
    }
    buttonPressed[index] = false;
  }
}

void chooseStartPlayer () {
  int trueIndices[NUM_SEATS];
  int numTrue = 0;

  for (int i = 0; i < NUM_SEATS; i++) {
    if (occupiedSeats[i]) {
      trueIndices[numTrue] = i;
      numTrue++;
    }
  }

  // If no true values were found, return -1
  if (numTrue == 0) {
    currentPlayer = -1;
  }

  // Choose a random index from the trueIndices array
  int randomIndex = random(numTrue);
  currentPlayer = trueIndices[randomIndex];

  *seatLEDs[currentPlayer] = colorGreen;
  FastLED.show();
}

void goToNextPlayer () {
  Serial.println("Going to next player");
  
  int previousPlayer = currentPlayer;

  int loopCounter = 0;
  do {
    currentPlayer = (previousPlayer + 1) % NUM_SEATS;
    loopCounter++;
  } while (!occupiedSeats[currentPlayer] && loopCounter < NUM_SEATS);

  if (loopCounter == NUM_SEATS) {
    Serial.println("No players found");
  }

  int startIndex = seatIndices[previousPlayer];
  int endIndex = seatIndices[currentPlayer];

  // If the end index is less than the start index, it means we've wrapped around to the start of the strip
  if (endIndex < startIndex) {
    endIndex += NUM_LEDS;
  }

  // Create the chasing light effect
  for (int i = startIndex; i < endIndex; i++) {
    // Turn on the LED at the current index
    leds[i % NUM_LEDS] = colorYellow;

    // Update the LED strip
    FastLED.show();

    // Turn off the LED at the current index
    leds[i % NUM_LEDS] = colorBlack;

    // Delay to slow down the effect
    delay(25);
  }

  // Make sure the LED at the current player's seat is left on
  *seatLEDs[currentPlayer] = colorGreen;
  FastLED.show();
}
