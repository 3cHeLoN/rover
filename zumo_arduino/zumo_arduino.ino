#include <Zumo32U4.h>

Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;
Zumo32U4LineSensors lineSensors;

const uint16_t maxSpeed = 300;
int16_t lastError = 0;
bool following = false;
bool agm_detected = false;

#define NUM_SENSORS 5
unsigned int lineSensorValues[NUM_SENSORS];

int motor_speed_left = 0;
int motor_speed_right = 0;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  while (!Serial1) {
    delay(100);
  }

  lineSensors.initFiveSensors();
}

void calibrateSensors()
{

  // Wait 1 second and then begin automatic sensor calibration
  // by rotating in place to sweep the sensors over the line
  delay(1000);
  for (uint16_t i = 0; i < 120; i++)
  {
    if (i > 30 && i <= 90)
    {
      motors.setSpeeds(-200, 200);
    }
    else
    {
      motors.setSpeeds(200, -200);
    }

    lineSensors.calibrate();
  }
  motors.setSpeeds(0, 0);
}

void drive()
{
  // Get the position of the line.  Note that we *must* provide
  // the "lineSensorValues" argument to readLine() here, even
  // though we are not interested in the individual sensor
  // readings.
  int16_t position = lineSensors.readLine(lineSensorValues);
  if (lineSensorValues[0] > 50 & lineSensorValues[4] > 50) {
    // above ground marker detected
    if (~agm_detected) {
      buzzer.playFrequency(3000, 1000, 10);
      agm_detected = true;
    }
  }
  else if (agm_detected) {
    Serial1.write('@');
    agm_detected = false;
  }

  // Our "error" is how far we are away from the center of the
  // line, which corresponds to position 2000.
  int16_t error = position - 2000;

  // Get motor speed difference using proportional and derivative
  // PID terms (the integral term is generally not very useful
  // for line following).  Here we are using a proportional
  // constant of 1/4 and a derivative constant of 6, which should
  // work decently for many Zumo motor choices.  You probably
  // want to use trial and error to tune these constants for your
  // particular Zumo and line course.
  int16_t speedDifference = error / 4 + 6 * (error - lastError);

  lastError = error;

  // Get individual motor speeds.  The sign of speedDifference
  // determines if the robot turns left or right.
  int16_t leftSpeed = (int16_t)maxSpeed + speedDifference;
  int16_t rightSpeed = (int16_t)maxSpeed - speedDifference;

  // Constrain our motor speeds to be between 0 and maxSpeed.
  // One motor will always be turning at maxSpeed, and the other
  // will be at maxSpeed-|speedDifference| if that is positive,
  // else it will be stationary.  For some applications, you
  // might want to allow the motor speed to go negative so that
  // it can spin in reverse.
  leftSpeed = constrain(leftSpeed, 0, (int16_t)maxSpeed);
  rightSpeed = constrain(rightSpeed, 0, (int16_t)maxSpeed);

  motors.setSpeeds(leftSpeed, rightSpeed);
}


void loop() {

  if (Serial1.available() >= 3)
  {
    uint8_t c1 = Serial1.read();
    uint8_t c2 = Serial1.read();
    motor_speed_left = map(c1, 0, 255, -400, 400);
    motor_speed_right = map(c2, 0, 255, -400, 400);
    motors.setSpeeds(motor_speed_left, motor_speed_right);
    char buffer[10];
    Serial1.readBytesUntil('\n', buffer, 10);
  }
}

void test() {
  int motor_speed_1;
  int motor_speed_2;
  if (0) {
    //Serial1.print(motor_speed_1);
    //Serial1.println(motor_speed_2);
    //int motor_speed_1 = Serial1.parseInt();
    //int motor_speed_2 = Serial1.parseInt();

    //Serial1.print("m1: ");
    //Serial1.print(motor_speed_1);
    //Serial1.print(", m2: ");
    //Serial1.println(motor_speed_2);

    // Set motor speeds
    if (motor_speed_1 < 500) {
      motors.setSpeeds(motor_speed_1, motor_speed_2);
    }
    // honk 1 start
    else if (motor_speed_1 == 500) {
      if (buzzer.isPlaying() == 0)
        buzzer.playFrequency(4000, 2000, 12);
    }
    // stop honk
    else if (motor_speed_1 == 501) {
      buzzer.stopPlaying();
    }
    // honk 2 start
    else if (motor_speed_1 == 502) {
      if (buzzer.isPlaying() == 0)
        buzzer.playFrequency(1000, 2000, 12);
    }
    // disable leds
    else if (motor_speed_1 == 503) {
      ledRed(0);
      ledYellow(0);
      ledGreen(0);
    }
    // enable leds
    else if (motor_speed_1 == 504) {
      ledRed(1);
      ledYellow(1);
      ledGreen(1);
    }
    // calibrate line follower
    else if (motor_speed_1 == 505) {
      calibrateSensors();
    }
    else if (motor_speed_1 == 506) {
      if (following) {
        following = false;
      }
      else {
        following = true;
      }
    }

    // flush buffer
    //char input[80];
    //Serial1.readBytesUntil('\n', input, 80);
    Serial1.flush();
  }

  if (following) {
    drive();
  }

}