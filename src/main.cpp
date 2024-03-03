#include <Arduino.h>
#include <FastLED.h>

// LED settings
#define LED_PIN 2
#define NUM_LEDS 200
#define NUM_SEATS 8
#define seat1 30
#define seat2 52
#define seat3 75
#define seat4 96
#define seat5 119
#define seat6 142
#define seat7 164
#define seat8 187
int seatIndices[NUM_SEATS] = {seat1, seat2, seat3, seat4, seat5, seat6, seat7, seat8};
CRGB leds[NUM_LEDS];
CRGB* seatLEDs[NUM_SEATS] = {&leds[seat1], &leds[seat2], &leds[seat3], &leds[seat4], &leds[seat5], &leds[seat6], &leds[seat7], &leds[seat8]};
CRGB colorGreen;
CRGB colorYellow;
CRGB colorRed;
CRGB colorBlack;
CRGB colorBlue;

// Button settings
const int buttonPins[NUM_SEATS] = {3, 4, 5, 6, 7, 8, 9, 10};
#define LONG_PRESS_TIME 1000
static unsigned long buttonPressTime[NUM_SEATS];
static bool buttonPressed[NUM_SEATS] = {false};
static bool buttonLongPressTriggered[NUM_SEATS] = {false};

// State variables
bool occupiedSeats[NUM_SEATS] = {false, false, false, false, false, false, false, false};
bool setupMode = false;
int currentPlayer;

// Function prototypes
void printBoolArray(bool array[], int size);
void resetLeds(CRGB flashColor);
void toggleSetupMode();
void setOccupiedSeat(int number, bool value);
void resetOccupiedSeats();
void buttonHandler(int index);
void chooseStartPlayer();
void trainEffect(int restIndex);
void goToNextPlayer ();

void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  colorGreen = CRGB::Green;
  colorYellow = CRGB::Yellow;
  colorRed = CRGB::Red;
  colorBlack = CRGB::Black;
  colorBlue = CHSV(170, 255, 128);

  resetLeds(colorBlack);
  resetOccupiedSeats();
}

void loop() {
  buttonHandler(0);
  buttonHandler(1);
  buttonHandler(2);
  buttonHandler(3);
  buttonHandler(4);
  buttonHandler(5);
  buttonHandler(6);
  buttonHandler(7);
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
        Serial.print("Button ");
        Serial.print(index + 1);
        toggleSetupMode();
        buttonLongPressTriggered[index] = true;
      }
    }
  } else if (buttonState == LOW && buttonPressed[index]) {
    if (!buttonLongPressTriggered[index]) {
      Serial.println("Short press detected");
      Serial.print("Button ");
      Serial.print(index + 1);
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

  // Start from the end of the array and move towards the start
  for (int i = NUM_SEATS - 1; i >= 0; i--) {
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

  trainEffect(seatIndices[currentPlayer]);
  FastLED.show();
}

void trainEffect(int restIndex) {
  const int TRAIN_LENGTH = 3; // Length of the light train
  const int MIN_LAPS = 3; // Minimum number of laps
  const int MAX_LAPS = 6; // Maximum number of laps
  const int START_DELAY = 5; // Starting delay in milliseconds
  const int END_DELAY = 25; // Ending delay in milliseconds

  // Calculate the total number of steps
  int totalSteps = random(MIN_LAPS, MAX_LAPS + 1) * NUM_LEDS;

  // Calculate the rest position in steps
  int restPosition = totalSteps - restIndex - TRAIN_LENGTH;

  // Loop over each step
  for (int i = 0; i <= restPosition; i++) {
    // Calculate the current delay
    int currentDelay = map(i, 0, restPosition, START_DELAY, END_DELAY);

    // Turn off all LEDs
    fill_solid(leds, NUM_LEDS, colorBlack);

    // Turn on the LEDs in the train
    for (int j = 0; j < TRAIN_LENGTH; j++) {
      if (i - j >= 0) {
        leds[(NUM_LEDS - (i - j) % NUM_LEDS) - 1] = colorYellow; // Change here for reverse direction
      }
    }

    // Update the LED strip
    FastLED.show();
    delay(currentDelay);
  }

  // Turn off all LEDs
  fill_solid(leds, NUM_LEDS, colorBlack);
  FastLED.show();

  // Turn on the LED at the rest position
  leds[restIndex] = colorGreen;
  FastLED.show();
}


void goToNextPlayer () {
  Serial.println("Going to next player");
  
  int previousPlayer = currentPlayer;

  int loopPlayer = previousPlayer;
  for (int i = 0; i < NUM_SEATS; i++) {
    int nextPlayer = (loopPlayer - 1 + NUM_SEATS) % NUM_SEATS; // Change here for reverse direction
    if (occupiedSeats[nextPlayer]) {
      currentPlayer = nextPlayer;
      break;
    }
    loopPlayer = nextPlayer;
  }

  int startIndex = seatIndices[previousPlayer];
  int endIndex = seatIndices[currentPlayer];

  // If the end index is greater than the start index, it means we've wrapped around to the start of the strip
  if (endIndex > startIndex) {
    startIndex += NUM_LEDS;
  }

  // Create the chasing light effect
  for (int i = startIndex; i > endIndex; i--) { // Change here for reverse direction
    // Turn on the LED at the current index
    leds[i % NUM_LEDS] = colorGreen;

    // Update the LED strip
    FastLED.show();

    // Turn off the LED at the current index
    leds[i % NUM_LEDS] = colorBlack;

    // Delay to slow down the effect
    delay(10);
  }

  // Make sure the LED at the current player's seat is left on
  *seatLEDs[currentPlayer] = colorGreen;
  FastLED.show();
}

void printBoolArray(bool array[], int size) {
  for (int i = 0; i < size; i++) {
    Serial.print("Value at index ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(array[i]);
  }
}
