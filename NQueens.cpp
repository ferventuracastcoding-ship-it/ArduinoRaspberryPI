/*
  ============================================================
       N-QUEENS LED BOARD VISUALIZER
       Arduino + Raspberry Pi Compatible Concept
  ============================================================

  BOARD:
      8 x 8 LED matrix / NeoPixel grid

  BUTTONS:
      1 = Reset
      2 = Random Queens
      3 = Solve
      4 = Next Solution
      5 = Backtracking
      6 = Conflict Heatmap
      7 = Checkerboard
      8 = Threat Lines
      9 = Speed
     10 = Auto Solve

  IMPORTANT:
  This sketch assumes an addressable LED board such as
  WS2812B / NeoPixel.

  Change LED_PIN and button pins for your hardware.
  ============================================================
*/

#include <Adafruit_NeoPixel.h>

#define LED_PIN 6

#define BOARD_SIZE 8
#define NUM_LEDS 64

Adafruit_NeoPixel leds(
  NUM_LEDS,
  LED_PIN,
  NEO_GRB + NEO_KHZ800
);

// ------------------------------------------------------------
// BUTTON PINS
// ------------------------------------------------------------

const int buttonPins[10] = {
  2, 3, 4, 5, 7,
  8, 9, 10, 11, 12
};

// ------------------------------------------------------------
// COLORS
// ------------------------------------------------------------

uint32_t BLACK;
uint32_t WHITE;
uint32_t BLUE;
uint32_t GREEN;
uint32_t RED;
uint32_t YELLOW;
uint32_t PURPLE;
uint32_t CYAN;
uint32_t ORANGE;

// ------------------------------------------------------------
// BOARD STATE
// ------------------------------------------------------------

int queens[BOARD_SIZE];

bool solving = false;
bool autoSolve = false;
bool heatmapMode = false;
bool threatMode = false;

int animationSpeed = 150;

unsigned long lastStep = 0;

// Solution storage
int solutionCount = 0;
int currentSolution = 0;

int solutions[92][BOARD_SIZE];

// ------------------------------------------------------------
// LED INDEX
// ------------------------------------------------------------

int indexLED(int row, int col) {

  // Serpentine LED arrangement
  if (row % 2 == 0) {
    return row * BOARD_SIZE + col;
  }

  return row * BOARD_SIZE +
         (BOARD_SIZE - 1 - col);
}

// ------------------------------------------------------------
// SET LED
// ------------------------------------------------------------

void setCell(
  int row,
  int col,
  uint32_t color
) {

  if (
    row < 0 ||
    row >= BOARD_SIZE ||
    col < 0 ||
    col >= BOARD_SIZE
  ) {
    return;
  }

  leds.setPixelColor(
    indexLED(row, col),
    color
  );
}

// ------------------------------------------------------------
// CLEAR BOARD
// ------------------------------------------------------------

void clearBoard() {

  for (int i = 0; i < NUM_LEDS; i++) {
    leds.setPixelColor(i, BLACK);
  }

  leds.show();
}

// ------------------------------------------------------------
// CHECK QUEEN CONFLICT
// ------------------------------------------------------------

bool conflict(
  int row,
  int col
) {

  for (int r = 0; r < row; r++) {

    int c = queens[r];

    if (c == col) {
      return true;
    }

    if (abs(c - col) == abs(r - row)) {
      return true;
    }
  }

  return false;
}

// ------------------------------------------------------------
// DRAW CHESSBOARD
// ------------------------------------------------------------

void drawBoard() {

  clearBoard();

  for (int row = 0; row < BOARD_SIZE; row++) {

    for (int col = 0; col < BOARD_SIZE; col++) {

      if ((row + col) % 2 == 0) {
        setCell(row, col, WHITE);
      } else {
        setCell(row, col, BLUE);
      }
    }
  }

  leds.show();
}

// ------------------------------------------------------------
// DRAW QUEENS
// ------------------------------------------------------------

void drawQueens() {

  drawBoard();

  for (int row = 0; row < BOARD_SIZE; row++) {

    int col = queens[row];

    if (col >= 0) {

      if (conflict(row, col)) {
        setCell(row, col, RED);
      } else {
        setCell(row, col, YELLOW);
      }
    }
  }

  leds.show();
}

// ------------------------------------------------------------
// RANDOM QUEENS
// ------------------------------------------------------------

void randomQueens() {

  for (int i = 0; i < BOARD_SIZE; i++) {

    queens[i] =
      random(0, BOARD_SIZE);
  }

  drawQueens();
}

// ------------------------------------------------------------
// CLEAR QUEENS
// ------------------------------------------------------------

void resetQueens() {

  for (int i = 0; i < BOARD_SIZE; i++) {
    queens[i] = -1;
  }

  solving = false;

  drawBoard();
}

// ------------------------------------------------------------
// SAVE SOLUTION
// ------------------------------------------------------------

void saveSolution() {

  if (solutionCount >= 92) {
    return;
  }

  for (int i = 0; i < BOARD_SIZE; i++) {
    solutions[solutionCount][i] =
      queens[i];
  }

  solutionCount++;
}

// ------------------------------------------------------------
// GENERATE ALL SOLUTIONS
// ------------------------------------------------------------

void generateSolutions(int row) {

  if (row == BOARD_SIZE) {

    saveSolution();

    return;
  }

  for (
    int col = 0;
    col < BOARD_SIZE;
    col++
  ) {

    if (!conflict(row, col)) {

      queens[row] = col;

      generateSolutions(row + 1);

      queens[row] = -1;
    }
  }
}

// ------------------------------------------------------------
// DISPLAY SOLUTION
// ------------------------------------------------------------

void displaySolution(int number) {

  if (
    number < 0 ||
    number >= solutionCount
  ) {
    return;
  }

  for (int i = 0; i < BOARD_SIZE; i++) {

    queens[i] =
      solutions[number][i];
  }

  drawQueens();
}

// ------------------------------------------------------------
// CONFLICT HEATMAP
// ------------------------------------------------------------

void drawHeatmap() {

  clearBoard();

  for (
    int row = 0;
    row < BOARD_SIZE;
    row++
  ) {

    for (
      int col = 0;
      col < BOARD_SIZE;
      col++
    ) {

      int threats = 0;

      for (
        int r = 0;
        r < BOARD_SIZE;
        r++
      ) {

        if (queens[r] < 0) {
          continue;
        }

        int c = queens[r];

        if (c == col) {
          threats++;
        }

        if (
          abs(c - col) ==
          abs(r - row)
        ) {
          threats++;
        }
      }

      if (threats == 0) {
        setCell(row, col, GREEN);
      }

      else if (threats == 1) {
        setCell(row, col, YELLOW);
      }

      else if (threats == 2) {
        setCell(row, col, ORANGE);
      }

      else {
        setCell(row, col, RED);
      }
    }
  }

  leds.show();
}

// ------------------------------------------------------------
// THREAT LINES
// ------------------------------------------------------------

void drawThreatLines() {

  drawBoard();

  for (
    int row = 0;
    row < BOARD_SIZE;
    row++
  ) {

    int col = queens[row];

    if (col < 0) {
      continue;
    }

    // Queen
    setCell(row, col, PURPLE);

    // Horizontal
    for (int c = 0; c < BOARD_SIZE; c++) {
      if (c != col) {
        setCell(row, c, RED);
      }
    }

    // Vertical
    for (int r = 0; r < BOARD_SIZE; r++) {
      if (r != row) {
        setCell(r, col, RED);
      }
    }

    // Diagonals
    for (int r = 0; r < BOARD_SIZE; r++) {

      for (int c = 0; c < BOARD_SIZE; c++) {

        if (
          abs(c - col) ==
          abs(r - row)
        ) {

          setCell(r, c, ORANGE);
        }
      }
    }
  }

  leds.show();
}

// ------------------------------------------------------------
// CHECKERBOARD ANIMATION
// ------------------------------------------------------------

void checkerAnimation() {

  for (int frame = 0; frame < 4; frame++) {

    for (
      int row = 0;
      row < BOARD_SIZE;
      row++
    ) {

      for (
        int col = 0;
        col < BOARD_SIZE;
        col++
      ) {

        if (
          (row + col + frame) % 2 == 0
        ) {
          setCell(row, col, CYAN);
        }

        else {
          setCell(row, col, BLACK);
        }
      }
    }

    leds.show();

    delay(250);
  }

  drawQueens();
}

// ------------------------------------------------------------
// BACKTRACK VISUALIZER
// ------------------------------------------------------------

bool solveStep(int row) {

  if (row == BOARD_SIZE) {
    return true;
  }

  for (
    int col = 0;
    col < BOARD_SIZE;
    col++
  ) {

    queens[row] = col;

    // Show current attempt
    drawQueens();

    delay(animationSpeed);

    if (!conflict(row, col)) {

      if (solveStep(row + 1)) {
        return true;
      }
    }

    // Backtrack
    setCell(row, col, RED);

    leds.show();

    delay(animationSpeed);

    queens[row] = -1;
  }

  return false;
}

// ------------------------------------------------------------
// START SOLVER
// ------------------------------------------------------------

void startSolver() {

  resetQueens();

  solving = true;

  solveStep(0);

  solving = false;
}

// ------------------------------------------------------------
// NEXT SOLUTION
// ------------------------------------------------------------

void nextSolution() {

  currentSolution++;

  if (
    currentSolution >= solutionCount
  ) {
    currentSolution = 0;
  }

  displaySolution(currentSolution);
}

// ------------------------------------------------------------
// SPEED CONTROL
// ------------------------------------------------------------

void changeSpeed() {

  animationSpeed -= 25;

  if (animationSpeed < 25) {
    animationSpeed = 250;
  }

  for (int i = 0; i < 3; i++) {

    leds.fill(CYAN);

    leds.show();

    delay(100);

    clearBoard();

    delay(100);
  }

  drawQueens();
}

// ------------------------------------------------------------
// AUTO SOLVER
// ------------------------------------------------------------

void autoSolveAnimation() {

  autoSolve = true;

  for (
    int s = 0;
    s < solutionCount;
    s++
  ) {

    displaySolution(s);

    delay(300);
  }

  autoSolve = false;
}

// ------------------------------------------------------------
// BUTTON HANDLER
// ------------------------------------------------------------

void handleButton(int button) {

  switch (button) {

    // 1
    case 0:
      resetQueens();
      break;

    // 2
    case 1:
      randomQueens();
      break;

    // 3
    case 2:
      startSolver();
      break;

    // 4
    case 3:
      nextSolution();
      break;

    // 5
    case 4:
      startSolver();
      break;

    // 6
    case 5:
      heatmapMode = !heatmapMode;

      if (heatmapMode) {
        drawHeatmap();
      } else {
        drawQueens();
      }

      break;

    // 7
    case 6:
      checkerAnimation();
      break;

    // 8
    case 7:
      threatMode = !threatMode;

      if (threatMode) {
        drawThreatLines();
      } else {
        drawQueens();
      }

      break;

    // 9
    case 8:
      changeSpeed();
      break;

    // 10
    case 9:
      autoSolveAnimation();
      break;
  }
}

// ------------------------------------------------------------
// SETUP
// ------------------------------------------------------------

void setup() {

  Serial.begin(115200);

  leds.begin();

  leds.setBrightness(80);

  BLACK =
    leds.Color(0, 0, 0);

  WHITE =
    leds.Color(30, 30, 30);

  BLUE =
    leds.Color(0, 0, 80);

  GREEN =
    leds.Color(0, 100, 20);

  RED =
    leds.Color(120, 0, 0);

  YELLOW =
    leds.Color(120, 80, 0);

  PURPLE =
    leds.Color(80, 0, 120);

  CYAN =
    leds.Color(0, 100, 120);

  ORANGE =
    leds.Color(120, 40, 0);

  // Buttons
  for (int i = 0; i < 10; i++) {

    pinMode(
      buttonPins[i],
      INPUT_PULLUP
    );
  }

  // Initialize board
  resetQueens();

  randomSeed(
    analogRead(A0)
  );

  // Generate all 92
  // standard 8-Queens solutions

  resetQueens();

  solutionCount = 0;

  generateSolutions(0);

  Serial.print("Solutions: ");
  Serial.println(solutionCount);

  drawBoard();
}

// ------------------------------------------------------------
// LOOP
// ------------------------------------------------------------

void loop() {

  for (int i = 0; i < 10; i++) {

    if (
      digitalRead(buttonPins[i])
      == LOW
    ) {

      handleButton(i);

      // Debounce
      delay(250);

      while (
        digitalRead(buttonPins[i])
        == LOW
      ) {
        delay(10);
      }
    }
  }
}
