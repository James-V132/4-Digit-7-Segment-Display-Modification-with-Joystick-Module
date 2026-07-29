#include "SevSeg.h"
SevSeg sevseg;

int num = 0;
int DP = 0;
// Relate to the display 

int x = A0;
int y = A1;
int button = 13;
// Relate to the wiring 

bool first_x = true;
bool first_y = true;
int speed = 500;
int speed_up_y_up = 0;
int speed_up_y_down = 0;
int last_time = 0;
// Relate to the speed of the increment/decrement and its change rate

bool x_steady = true;
bool y_steady = true;
int max_steady_y = 800;
int min_steady_y = 700;
int max_steady_x = 800;
int min_steady_x = 700;
// Relate to the joystick's steady position

void setup() {
  Serial.begin(9600);

  byte numDigits = 4;
  byte digitPins[] = {9, 10, 11, 12};
  byte segmentPins[] = {1, 2, 3, 4, 5, 6, 7, 8};
  bool resistorsOnSegments = false; // 'false' means resistors are on digit pins
  byte hardwareConfig = COMMON_CATHODE; // See README.md for options
  bool updateWithDelays = false; // Default 'false' is Recommended
  bool leadingZeros = true; // Use 'true' if you'd like to keep the leading zeros
  bool disableDecPoint = false; // Use 'true' if your decimal point doesn't exist or isn't connected. Then, you only need to specify 7 segmentPins[]

  sevseg.begin(hardwareConfig, numDigits, digitPins, segmentPins, resistorsOnSegments,
  updateWithDelays, leadingZeros, disableDecPoint);

  pinMode(x, INPUT);
  pinMode(y, INPUT);
  pinMode(button, INPUT_PULLUP);
}

void button_push() // Pushing the joystick button resets the digits to 0
{
  if(digitalRead(button) == 0)
  {
    num = 0;
  }
}

void loop() {
  sevseg.setNumber(num, DP);
  int start_time = millis();
  x_steady = (max_steady_x > analogRead(x)) && (analogRead(x) > min_steady_x);
  y_steady = (max_steady_y > analogRead(y)) && (analogRead(y) > min_steady_y);
  button_push();

  // Serial.print("Steady?: "); 
  // Serial.println(x_steady);
  // Serial.print("X Cord: ");
  // Serial.println(analogRead(x));
  // Serial.print("Y Cord: ");
  // Serial.println(analogRead(y));
  // Serial.println();
  // Debug info

  while(!(x_steady)) // Moves the decimal point
  {
    start_time = millis();
    button_push();
    if(analogRead(x) < min_steady_x) // Moves the decimal point rightward as the joystick is pushed rightward
    {
      if(first_x) // For the first time the joystick is pushed, the decimal point is immediately moved rightward
      {
        first_x = false;
        last_time = start_time;
        DP <= 0 ? DP = 3 : DP--;
      }
      else
      {
        if(start_time - last_time >= 750) // If the joystick is still held rightward, the decimal moves rightward every second
        {
          last_time = start_time;
          DP <= 0 ? DP = 3 : DP--;
        }
      }
    }

    if(analogRead(x) > max_steady_x) // Moves the decimal point leftward as the joystick is pushed leftward
    {
      if(first_x) // For the first time the joystick is pushed, the decimal point is immediately moved leftward
      {
        first_x = false;
        last_time = start_time;
        DP >= 3 ? DP = 0 : DP++;
      }
      else
      {
        if(start_time - last_time >= 750) // If the joystick is still held leftward, the decimal moves leftward every second
        {
          last_time = start_time;
          DP >= 3 ? DP = 0 : DP++;
        }
      }
    }

    x_steady = (max_steady_x > analogRead(x)) && (analogRead(x) > min_steady_x);
    sevseg.setNumber(num, DP);
    sevseg.refreshDisplay();
  }

  while(!(y_steady)) // Increments/Decrements the digit selected
  {
    start_time = millis();
    int num_to_change = (int)(num / pow(10, DP)) % 10; // Find the number at the current decimal point position
    button_push();
    if(analogRead(y) < min_steady_y) // Decrements the digit as the joystick is pushed downward
    {
      speed_up_y_up = 0;

      if(first_y) // For the first time the joystick is pushed, the digit is immediately decremented
      {
        speed_up_y_down++;
        first_y = false;
        last_time = start_time;

        num_to_change <= 0 ? (num += (9 * pow(10, DP))) : (num -= pow(10,DP));
      }
      else
      {
        speed_up_y_down >= 3 ? speed = 250 : speed = 500;
        if(start_time - last_time >= speed) // If the joystick is still held downward, the digit decrements every second
        {
          speed_up_y_down++;
          last_time = start_time;

          num_to_change <= 0 ? (num += (9 * pow(10, DP))) : (num -= pow(10,DP));
        }
      }
    }

    if(analogRead(y) > max_steady_y) // Increments the digit as the joystick is pushed upward
    {
      speed_up_y_down = 0;

      if(first_y) // For the first time the joystick is pushed, the digit is immediately incremented
      {
        speed_up_y_up++;
        first_y = false;
        last_time = start_time;

        num_to_change >= 9 ? (num -= (9 * pow(10, DP))) : (num += pow(10,DP));
      }
      else
      {
        speed_up_y_up >= 3 ? speed = 250 : speed = 500;
        if(start_time - last_time >= speed) // If the joystick is still held upward, the digit increments every second
        {
          speed_up_y_up++;
          last_time = start_time;

          num_to_change >= 9 ? (num -= (9 * pow(10, DP))) : (num += pow(10,DP));
        }
      }
    }
    y_steady = (max_steady_y > analogRead(y)) && (analogRead(y) > min_steady_y);
    sevseg.setNumber(num, DP);
    sevseg.refreshDisplay();
  }

  speed_up_y_up = 0;
  speed_up_y_down = 0;
  first_y = true;
  first_x = true;
  sevseg.refreshDisplay();
}