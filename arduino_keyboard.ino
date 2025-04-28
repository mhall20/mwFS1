#include <Keyboard.h>
#include <Mouse.h>

// Create columns
const int cols[] = {8, 9, 7, 14, 15, A0, A1};
const int numCols = sizeof(cols) / sizeof(cols[0]);

// Create rows
const int rows[] = {16, 10};
const int numRows = sizeof(rows) / sizeof(rows[0]);

// Keyboard button mapping (using keyboard keys as placeholder)
const int keyMap[numCols][numRows] = {
  {KEY_LEFT_ARROW, 0},
  {KEY_DOWN_ARROW, 0},
  {KEY_RIGHT_ARROW, KEY_UP_ARROW},
  {'x', 'a'},
  {'y', 'b'},
  {'e', 'r'},
  {'q', 't'}
};

// Button state tracking - makes it so buttons are only counted as being pressed once
bool previousButtonState[numCols][numRows] = {false};
bool currentButtonState[numCols][numRows] = {false};

// Track arrow key states separately
bool upPressed = false;
bool downPressed = false;
bool leftPressed = false;
bool rightPressed = false;

// Start, stop, select buttons
const int extraButtons[] = {4, 5, 6};
const int numExtraButtons = sizeof(extraButtons) / sizeof(extraButtons[0]);
bool previousExtraButtonState[numExtraButtons] = {false};
bool currentExtraButtonState[numExtraButtons] = {false};

void setup() {
  Serial.begin(9600);  // <-- Added to enable Serial printing
  Keyboard.begin();
  Mouse.begin();

  // Set extra buttons as INPUT_PULLUP
  for (int i = 0; i < numExtraButtons; i++) {
    pinMode(extraButtons[i], INPUT_PULLUP);
  }
  
  setMatrix();
  delay(1000); // Give the startup a little bit to go through
}

void loop() {
  scanMatrix();
  scanExtraButtons();
  handleArrowKeys();
  delay(10); // 10 millisecond delay to stop from overproducing inputs
}

// Scan through all rows and columns to establish buttons
void setMatrix() {
  for (int i = 0; i < numCols; i++) {
    pinMode(cols[i], INPUT_PULLUP);
  }

  for (int i = 0; i < numRows; i++) {
    pinMode(rows[i], OUTPUT);
    digitalWrite(rows[i], HIGH);
  }
}

// Scan the keyboard matrix and assign button presses to a key
void scanMatrix() {

  // Go through rows
  for (int row = 0; row < numRows; row++) {
    digitalWrite(rows[row], LOW);

    // Go through columns
    for (int col = 0; col < numCols; col++) {
      bool buttonPressed = (digitalRead(cols[col]) == LOW);
      int key = keyMap[col][row];

      // Only act when the button state changes
      if (buttonPressed != previousButtonState[col][row]) {
        if (buttonPressed) {
          printButtonPressed(col, row); // Print when button is pressed
          if (key != 0) {
            Keyboard.press(key);
          }
        } else {
          if (key != 0) {
            Keyboard.release(key);
          }
        }
        previousButtonState[col][row] = buttonPressed; // Update after handling
      }
    }

    // Assign that this button is currently being pressed
    digitalWrite(rows[row], HIGH);
  }
}

// Debugging function to print to serial the row, column, and name of the button that is being pressed
void printButtonPressed(int col, int row) {
  int key = keyMap[col][row];

  // Row and column print
  Serial.print("Button Pressed at (Col ");
  Serial.print(col);
  Serial.print(", Row ");
  Serial.print(row);
  Serial.print(") -> ");

  // Print what button is being pressed - created because some buttons were being detected as unkown keys, so I needed to see which they were
  if (key == 0) {
    Serial.println("No Key Assigned");
  } else if (key == KEY_LEFT_ARROW) {
    Serial.println("Left Arrow");
  } else if (key == KEY_RIGHT_ARROW) {
    Serial.println("Right Arrow");
  } else if (key == KEY_UP_ARROW) {
    Serial.println("Up Arrow");
  } else if (key == KEY_DOWN_ARROW) {
    Serial.println("Down Arrow");
  } else if (key >= 32 && key <= 126) { // printable ASCII
    Serial.print("Character: ");
    Serial.println((char)key);
  } else {
    Serial.print("Unknown Key (Code ");
    Serial.print(key);
    Serial.println(")");
  }
}

/* "SOCD clean" is required for tournament play,
    so this function cancels out opposite arrow inputs ex. up and down, and left and right */
void handleArrowKeys() {
  bool newUpPressed = currentButtonState[2][1];
  bool newDownPressed = currentButtonState[1][0];
  bool newLeftPressed = currentButtonState[0][1];
  bool newRightPressed = currentButtonState[2][0];

  // Up and Down clean
  if (newUpPressed && newDownPressed) {
    if (upPressed) {
      Keyboard.release(KEY_UP_ARROW);
      upPressed = false;
    }
    if (downPressed) {
      Keyboard.release(KEY_DOWN_ARROW);
      downPressed = false;
    }
  } else {
    if (newUpPressed != upPressed) {
      if (newUpPressed) {
        Keyboard.press(KEY_UP_ARROW);
      } else {
        Keyboard.release(KEY_UP_ARROW);
      }
      upPressed = newUpPressed; // Undo the canceling
    }
    
    if (newDownPressed != downPressed) {
      if (newDownPressed) {
        Keyboard.press(KEY_DOWN_ARROW);
      } else {
        Keyboard.release(KEY_DOWN_ARROW);
      }
      downPressed = newDownPressed; // Undo the canceling
    }
  }

  // Left and Right clean
  if (newLeftPressed && newRightPressed) {
    if (leftPressed) {
      Keyboard.release(KEY_LEFT_ARROW);
      leftPressed = false;
    }
    if (rightPressed) {
      Keyboard.release(KEY_RIGHT_ARROW);
      rightPressed = false;
    }
  } else {
    if (newLeftPressed != leftPressed) {
      if (newLeftPressed) {
        Keyboard.press(KEY_LEFT_ARROW);
      } else {
        Keyboard.release(KEY_LEFT_ARROW);
      }
      leftPressed = newLeftPressed; // Undo the canceling
    }
    
    if (newRightPressed != rightPressed) {
      if (newRightPressed) {
        Keyboard.press(KEY_RIGHT_ARROW);
      } else {
        Keyboard.release(KEY_RIGHT_ARROW);
      }
      rightPressed = newRightPressed; // Undo the canceling
    }
  }
}

// Function to create the start, options, and home button
void scanExtraButtons() {

  // loop through the extra buttons and add them to the list
  for (int i = 0; i < numExtraButtons; i++) {
    int pin = extraButtons[i];
    bool pressed = (digitalRead(pin) == LOW);

    currentExtraButtonState[i] = pressed;

    // Check for previous button state so that when you press it, the input only happens once
    if (currentExtraButtonState[i] != previousExtraButtonState[i]) {
      if (pressed) {
        // Button just pressed
        switch (pin) {
          case 6:
            Keyboard.press(KEY_RETURN);
            Serial.println("Extra Button: Enter"); // Debugging
            break;
          case 5:
            Keyboard.press(KEY_BACKSPACE);
            Serial.println("Extra Button: Backspace"); // Debugging
            break;
          case 4:
            Keyboard.press(KEY_LEFT_GUI);
            Serial.println("Extra Button: Windows Key"); // Debugging
            break;
        }
      } else {
        // Button just released
        switch (pin) {
          case 6:
            Keyboard.release(KEY_RETURN);
            break;
          case 5:
            Keyboard.release(KEY_BACKSPACE);
            break;
          case 4:
            Keyboard.release(KEY_LEFT_GUI);
            break;
        }
      }
    }

    previousExtraButtonState[i] = currentExtraButtonState[i];
  }
}
