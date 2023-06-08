#include "motor.h"
#include "sensor.h"
#include "movement.h"

// Extend To Array { speed, speed - 5, speed, speed - 5} max value's 255
#define SPEED 255

// value sensor is in black line
#define BLACK_VALUE_MAX 400
#define BLACK_VALUE_MIN 350

// Sensor Tracking Position -> { a0  a1  a2  a3  a4 }
#define SENSOR_A0 64
#define SENSOR_A1 55
#define SENSOR_A2 56
#define SENSOR_A3 57
#define SENSOR_A4 58

// Sensor Tracking Position -> { a5  a6  a7  a8  a9 }
#define SENSOR_A5 63
#define SENSOR_A6 59
#define SENSOR_A7 61
#define SENSOR_A8 62
#define SENSOR_A9 60

short a0_value, a1_value, a2_value, a3_value, a4_value;
short a5_value, a6_value, a7_value, a8_value, a9_value;

bool isMove = true;
bool isBack = false;
bool isSoi = false;

// Delay Time
unsigned long logTime = 0;
unsigned long delayTurn = 0;
unsigned long delaybeforeTurn = 0;

int cross = 0;
int soi_count = 0;
int finish_soi_count = 0;

Motor motor;
Movement movement;
movement.sensor = Sensor(255, 300);

void setup()
{
  Serial.begin(115200);

  motor.initMotor();

  pinMode(SENSOR_A0, INPUT);
  pinMode(SENSOR_A1, INPUT);
  pinMode(SENSOR_A2, INPUT);
  pinMode(SENSOR_A3, INPUT);
  pinMode(SENSOR_A4, INPUT);
  pinMode(SENSOR_A5, INPUT);
  pinMode(SENSOR_A6, INPUT);
  pinMode(SENSOR_A7, INPUT);
  pinMode(SENSOR_A8, INPUT);
  pinMode(SENSOR_A9, INPUT);
}

void loop()
{
  sensor_read();

  // CurrentTimeLine
  unsigned long currentTime = millis();

  if (isMove)
  {
    if (!isBack)
      balance_move();
    else
      balance_move_back();
  }

  if (!isMove)
  {
    if (cross == 0)
    {
      if (currentTime - delayTurn > 800)
      {
        if (isBlack(a2_value) || isBlack(a3_value))
        {
          isMove = true;
          delayTurn = 0;
          cross += 1;
        }
      }
      motor.move(SPEED, "left");
    }
    else if (cross == 1)
    {
      if (currentTime - delayTurn > 800)
      {
        if (isBlack(a1_value) || isBlack(a2_value))
        {
          soi_count += 1;
          delayTurn = currentTime;
          if (soi_count == 4)
          {
            isSoi = true;
            isMove = true;
            soi_count = 0;
            delayTurn = 0;
            cross += 1;
          }
        }
      }
      balance_slide('L');
    }
    else if (cross == 2 && !isBack)
    {
      if (currentTime - delayTurn > 800)
      {
        if (isBlack(a1_value) || isBlack(a2_value))
        {
          isMove = true;
          delayTurn = 0;
          cross += 1;
        }
      }
      motor.move(SPEED, "right");
    }
    else if (cross == 3)
    {
      if (currentTime - delayTurn > 800)
      {
        if (isBlack(a2_value) || isBlack(a3_value))
        {
          isMove = true;
          delayTurn = 0;
          cross += 1;
        }
      }
      motor.move(SPEED, "right");
    }
    else if (cross == 5)
    {
      if (currentTime - delayTurn > 800)
      {
        if (isBlack(a1_value) || isBlack(a2_value))
        {
          soi_count += 1;
          delayTurn = currentTime;
          if (soi_count == 4)
          {
            isSoi = true;
            isMove = true;
            soi_count = 0;
            delayTurn = 0;
            cross += 1;
          }
        }
      }
      balance_slide('L');
    }
    else if (cross == 6)
    {
      if (currentTime - delayTurn > 800)
      {
        if (isBlack(a2_value) || isBlack(a3_value))
        {
          isMove = true;
          delayTurn = 0;
          cross += 1;
        }
      }
      motor.move(SPEED, "right");
    }
  }

  if (isCenter())
  {
    if (cross == 0)
    {
      delay(400);
      if (delayTurn == 0)
      {
        isMove = false;
        delayTurn = currentTime;
      }
    }
    else if (cross == 1)
    {
      delay(300);
      if (delayTurn == 0)
      {
        isMove = false;
        delayTurn = currentTime;
      }
    }
    else if (cross == 4)
    {
      delay(200);
      balance_move();
      cross += 1;
    }
    else if (cross == 5)
    {
      delay(200);
      motor.move(SPEED, "right");
      delay(100);
      if (delayTurn == 0)
      {
        isMove = false;
        delayTurn = currentTime;
      }
    }
  }

  if (isBlack(a2_value) && isBlack(a3_value) && isBlack(a4_value))
  {
    if ((cross == 2 || cross == 3 || cross == 6) && finish_soi_count == 1)
    {
      delay(500);
      if (delayTurn == 0)
      {
        isMove = false;
        delayTurn = currentTime;
      }
    }
  }

  if (currentTime - logTime > 250)
  {
    log_sensor();
    logTime = currentTime;
  }

  delay(50);
}

bool isCenter()
{
  return (a0_value > 850 && isBlack(a1_value) && isBlack(a2_value) && isBlack(a3_value) && isBlack(a4_value));
}

bool isBlack(short sensor_value)
{
  return sensor_value > BLACK_VALUE_MIN;
}

void balance_move()
{
  // Apply the control output to the movement
  if (isBlack(a2_value))
    motor.move(SPEED, "front");
  else if (a0_value > 850 || isBlack(a1_value))
    motor.move(SPEED, "left");
  else if (isBlack(a3_value) || isBlack(a4_value))
    motor.move(SPEED, "right");
  else
  {
    if (isSoi)
    {
      if (isBack)
      {
        motor.move(SPEED, "front");
      }
      else
      {
        motor.move(0, "stop");
        delay(1000);
        motor.move(SPEED, "back");
        delay(500);
        isBack = true;
      }
    }
    else
      motor.move(0, "stop");
  }
}

void balance_move_back()
{
  if (isBlack(a2_value))
    motor.move(SPEED, "back");
  else if (isBlack(a1_value) || isBlack(a4_value))
    motor.move(SPEED, "back");
  else if (a0_value > 850 || isBlack(a3_value))
    motor.move(SPEED, "back");
  else
  {
    if (isSoi)
    {
      motor.move(SPEED, "front");
      delay(750);
      motor.move(SPEED, "right");
      delay(750);
      isMove = false;
      isSoi = false;
      isBack = false;
      finish_soi_count += 1;
      Serial.println("back");
    }
    else
      motor.move(0, "stop");
  }
}

void balance_slide(char direction)
{
  if (direction == 'L')
    if (isBlack(a5_value) || isBlack(a6_value))
      motor.move(SPEED, "sleft");
    else if (isBlack(a7_value) || isBlack(a8_value) || a9_value > 300)
      motor.move(100, "back");
    else
      motor.move(SPEED, "stop");

  if (direction == 'R')
    if (isBlack(a8_value) || a9_value > 300)
      motor.move(SPEED, "sright");
    else if (isBlack(a5_value) || isBlack(a6_value) || isBlack(a7_value))
      motor.move(150, "front");
    else
      motor.move(SPEED, "stop");
}

void sensor_read()
{
  a0_value = analogRead(SENSOR_A0);
  a1_value = analogRead(SENSOR_A1);
  a2_value = analogRead(SENSOR_A2);
  a3_value = analogRead(SENSOR_A3);
  a4_value = analogRead(SENSOR_A4);
  a5_value = analogRead(SENSOR_A5);
  a6_value = analogRead(SENSOR_A6);
  a7_value = analogRead(SENSOR_A7);
  a8_value = analogRead(SENSOR_A8);
  a9_value = analogRead(SENSOR_A9);
}

void log_sensor()
{
  Serial.print("A0: ");
  Serial.print(a0_value);
  Serial.print(" A1: ");
  Serial.print(a1_value);
  Serial.print(" A2: ");
  Serial.print(a2_value);
  Serial.print(" A3: ");
  Serial.print(a3_value);
  Serial.print(" A4: ");
  Serial.print(a4_value);
  Serial.println("");
  Serial.print("A5: ");
  Serial.print(a5_value);
  Serial.print(" A6: ");
  Serial.print(a6_value);
  Serial.print(" A7: ");
  Serial.print(a7_value);
  Serial.print(" A8: ");
  Serial.print(a8_value);
  Serial.print(" A9: ");
  Serial.print(a9_value);
  Serial.println("");
  Serial.println("");
  Serial.print("cross: ");
  Serial.print(cross);
  Serial.print(" soi_count: ");
  Serial.print(soi_count);
  Serial.println("");
  Serial.println("");
}