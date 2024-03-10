#include <Arduino.h>
#include <FastLED.h>

// LED settings
#define LED_PIN 2
#define NUM_LEDS 200
#define NUM_SEATS 8
#define seat1 30
#define seat2 52
#define seat3 74
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
int currentPlayer = -1;
unsigned long currentPlayerStartTime = 0;

// Function prototypes
void toggleSetupMode();
void setOccupiedSeat(int number, bool value);
void resetOccupiedSeats();
void buttonHandler(int index);
void chooseStartPlayer();
void singleLightEffect(int restIndex);
void allLightsOffEffect(int restIndex);
void goToNextPlayer();
void checkCurrentPlayerTime();

void setup() {
  Serial.begin(9600);

  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  colorGreen = CRGB::Green;
  colorYellow = CRGB::Yellow;
  colorRed = CRGB::Red;
  colorBlack = CRGB::Black;
  colorBlue = CHSV(170, 255, 128);

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = colorBlack;
    FastLED.show();
  }
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

  checkCurrentPlayerTime();
}

void toggleSetupMode() {
  setupMode = !setupMode;

  bool moreThanOneSeatOccupied = false;
  int occupiedCount = 0;
  for (size_t i = 0; i < sizeof(occupiedSeats)/sizeof(occupiedSeats[0]); i++) {
    if (occupiedSeats[i]) {
      occupiedCount++;
      if (occupiedCount > 1) {
        moreThanOneSeatOccupied = true;
        break;
      }
    }
  }

  if (setupMode) {
    resetOccupiedSeats();
    for (int i = 21; i < NUM_LEDS; i++) {
      leds[i] = colorBlue;
    }
    FastLED.show();
  } else if (!moreThanOneSeatOccupied) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = colorBlack;
      FastLED.show();
    }
  } else {
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
    currentPlayerStartTime = 0;
  }

  // Choose a random index from the trueIndices array
  int randomIndex = random(numTrue);
  currentPlayer = trueIndices[randomIndex];
  currentPlayerStartTime = millis();

  int randomChoice = random(0, 2); // Generate a random number (0 or 1)

  if (randomChoice == 0) {
    singleLightEffect(seatIndices[currentPlayer]);
  } else {
    allLightsOffEffect(seatIndices[currentPlayer]);
  }
  FastLED.show();
}

void singleLightEffect(int restIndex) {
  const int MIN_LAPS = 3; // Minimum number of laps
  const int MAX_LAPS = 6; // Maximum number of laps
  const int START_DELAY = 1; // Starting delay in milliseconds
  const int END_DELAY = 15; // Ending delay in milliseconds
  const int SKIP_INDEX = 21; // Index to skip to
  const int START_INDEX = 200; // Starting index
  const int HIDDEN_LEDS = 20; // Number of hidden LEDs

  // Calculate the total number of steps
  int totalLaps = random(MIN_LAPS, MAX_LAPS + 1);
  int totalSteps = totalLaps * (NUM_LEDS - HIDDEN_LEDS) + (START_INDEX - restIndex);

  // Loop over each step
  for (int i = 0; i < totalSteps; i++) {
    // Calculate the current delay
    int currentDelay = map(i, 0, totalSteps, START_DELAY, END_DELAY);

    // Turn off all LEDs
    fill_solid(leds, NUM_LEDS, colorBlack);

    // Calculate the LED index
    int ledIndex = START_INDEX - (i % (NUM_LEDS - HIDDEN_LEDS));
    if (ledIndex < SKIP_INDEX) {
      ledIndex += (NUM_LEDS - SKIP_INDEX);
    }

    // Turn on the LED at the calculated index
    leds[ledIndex] = colorGreen;

    // Update the LED strip
    FastLED.show();

    // Delay for the current delay
    delay(currentDelay);
  }

  // Turn off all LEDs
  fill_solid(leds, NUM_LEDS, colorBlack);

  // Turn on the LED at the rest position
  leds[restIndex] = colorGreen;

  // Update the LED strip
  FastLED.show();
}

void allLightsOffEffect(int restIndex) {
  const int START_DELAY = 1; // Starting delay in milliseconds
  const int END_DELAY = 15; // Ending delay in milliseconds
  const int SKIP_INDEX = 21; // Index to skip to
  const int START_INDEX = 200; // Starting index

  // Turn on all LEDs from 21 to 200
  for (int i = SKIP_INDEX; i <= START_INDEX; i++) {
    if (i < NUM_LEDS) {
      leds[i] = colorGreen;
    }
  }

  // Update the LED strip
  FastLED.show();

  // Create an array of LED indices
  int ledIndices[START_INDEX - SKIP_INDEX + 1];
  int count = 0;
  for (int i = SKIP_INDEX; i <= START_INDEX; i++) {
    if (i != restIndex) {
      ledIndices[count++] = i;
    }
  }

  // Shuffle the LED indices
  for (int i = 0; i < count; i++) {
    int randomIndex = random(0, count);
    int temp = ledIndices[i];
    ledIndices[i] = ledIndices[randomIndex];
    ledIndices[randomIndex] = temp;
  }

  // Loop over each LED index
  for (int i = 0; i < count; i++) {
    // Calculate the current delay
    int currentDelay = map(i, 0, count, START_DELAY, END_DELAY);

    // Turn off the LED at the current index
    leds[ledIndices[i]] = colorBlack;

    // Update the LED strip
    FastLED.show();

    // Delay for the current delay
    delay(currentDelay);
  }
}

void goToNextPlayer () {
  // Clear out all the LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = colorBlack;
  }

  // Light up the LED at the currentPlayer's seat
  if (currentPlayer >= 21) {
    *seatLEDs[currentPlayer] = colorGreen;
  }

  FastLED.show();

  int previousPlayer = currentPlayer;

  int loopPlayer = previousPlayer;
  for (int i = 0; i < NUM_SEATS; i++) {
    int nextPlayer = (loopPlayer - 1 + NUM_SEATS) % NUM_SEATS;
    if (occupiedSeats[nextPlayer]) {
      currentPlayer = nextPlayer;
      currentPlayerStartTime = millis();
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
    if (i % NUM_LEDS >= 21) {
      leds[i % NUM_LEDS] = colorGreen;

      // Update the LED strip
      FastLED.show();

      // Turn off the LED at the current index
      leds[i % NUM_LEDS] = colorBlack;
      delay(10);
    }

    // Delay to slow down the effect
  }

  // Make sure the LED at the current player's seat is left on
  if (currentPlayer >= 21) {
    *seatLEDs[currentPlayer] = colorGreen;
    FastLED.show();
  }
}

void checkCurrentPlayerTime() {
  if (currentPlayer == -1) {
    return;
  }
  
  unsigned long currentTime = millis();
  unsigned long timePassed = currentTime - currentPlayerStartTime;
  int secondsToWait = 60;

  int ledsToLight = timePassed / (secondsToWait * 1000) ; // Number of LEDs to light on either side

  if (ledsToLight > 10) {
    ledsToLight = 10;
  }

  // Get the LED index for the currentPlayer
  int currentPlayerLedIndex = seatIndices[currentPlayer];

  // Calculate the color based on ledsToLight
  int red = ledsToLight * 25.5; // Goes from 0 to 255 as ledsToLight goes from 0 to 10
  int green = 255 - red; // Goes from 255 to 0 as ledsToLight goes from 0 to 10
  CRGB color = CRGB(red, green, 0);

  // Light up the LED at the currentPlayer index
  if (currentPlayerLedIndex >= 21 && currentPlayerLedIndex < NUM_LEDS) {
    leds[currentPlayerLedIndex] = color;
  }

  // Light up ledsToLight LEDs on either side of the currentPlayer
  for (int i = 1; i <= ledsToLight; i++) {
    if (currentPlayerLedIndex - i >= 21) {
      leds[currentPlayerLedIndex - i] = color;
    }
    if (currentPlayerLedIndex + i < NUM_LEDS) {
      leds[currentPlayerLedIndex + i] = color;
    }
  }

  FastLED.show();
}
