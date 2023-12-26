// ROS
#include <ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/String.h>
#include <std_msgs/float32.h>

ros::NodeHandle nh;
geometry_msgs::Twist msg;
std_msgs::Float32 str_msg;
ros::Publisher pub("walker_test", &str_msg);

// ON/OFF Switch
#include <ezButton.h>
ezButton toggleSwitch(12);

// 2-Axis Joystick
const int SW_pin = 53;
const int X_pin = 0;
const int Y_pin = 1;
int xAxis_Reading = 0;
int yAxis_Reading = 0;
int Forward_SpeedReading = 0;
int Backward_SpeedReading = 0;
int TurnLeft_SpeedReading = 0;
int TurnRight_SpeedReading = 0;

const char* currentMovement;

// MDD10A Rev2.0 Motor Driver #1
const int MotorDriver1_Direction1 = 4;
const int MotorDriver1_Speed1 = 5;
const int MotorDriver1_Direction2 = 6;
const int MotorDriver1_Speed2 = 7;

// MDD10A Rev2.0 Motor Driver #2
const int MotorDriver2_Direction1 = 8;
const int MotorDriver2_Speed1 = 9;
const int MotorDriver2_Direction2 = 10;
const int MotorDriver2_Speed2 = 11;

// Servo Motor
#include <Servo.h>
Servo servoMotor;
int servo_pin = 13;

// Function Prototype - Movement
void Forward(int autoSpeed, int& manualSpeed, int state);
void Backward(int autoSpeed, int& manualSpeed, int state);
void TurnLeft(int autoSpeed, int& manualSpeed, int state);
void TurnRight(int autoSpeed, int& manualSpeed, int state);
void Stationary();

// ROSSERIAL - Call Back Function
void CallBackFunction(const geometry_msgs::Twist& cmd_vel){
  float linear_x = cmd_vel.linear.x;
  float angular_z = cmd_vel.angular.z;
  str_msg.data = angular_z;
  pub.publish(&str_msg);

  // Map the value to the Arduino's rotation range (30 to 255) (Motors won't move under 30)
  // int arduino_value = (int)((angular_z + 0.05672) / 0.11344 * 255);
  // Constrain the value to be within the valid range
  // arduino_value = constrain(arduino_value, 0, 1024);
  // int arduino_value = int(round((angular_z + 1) * 127.5));

  // int angular_zCONVERTED = int(round(angular_z * 1000));
  // int arduino_value = map(angular_zCONVERTED, -5672, 5672, 0, 255);

  int angular_z_speed = abs(int(round(angular_z * 10000)));
  angular_z_speed = map(angular_z_speed, 0, 5672, 30, 60);

  int linear_x_speed = abs(int(round(linear_x * 10000)));
  linear_x_speed = map(linear_x_speed, 0, 4000, 30, 80);
  // Serial.println(arduino_value);

  // str_msg.data = cmd_vel.angular.z;
  // pub.publish(&str_msg);

  if (angular_z >= 0.002 && angular_z <= 0.012) { TurnLeft(angular_z_speed, TurnLeft_SpeedReading, 0); }
  else if (angular_z <= -0.012 && angular_z >= -0.002) { TurnRight(angular_z_speed, TurnRight_SpeedReading, 0); }
  else if (linear_x >= 0.2) { Backward(linear_x_speed, Backward_SpeedReading, 0); }
  // if (angular_z >= 0.002 && angular_z <= 0.012) { TurnLeft(0, TurnLeft_SpeedReading, 1); }
  // else if (angular_z <= -0.012 && angular_z >= -0.002) { TurnRight(0, TurnRight_SpeedReading, 1); }
  // else if (linear_x >= 0.2) { Backward(linear_x_speed, Backward_SpeedReading, 1); }
  else if (linear_x < 0.2) 
  { 
    Forward_SpeedReading -= 15;
    Backward_SpeedReading -= 15;
    TurnLeft_SpeedReading -= 20;
    TurnRight_SpeedReading -= 20;
    if (Forward_SpeedReading < 16) { Forward_SpeedReading = 0; }
    if (Backward_SpeedReading < 16) { Backward_SpeedReading = 0; }
    if (TurnLeft_SpeedReading < 16) { TurnLeft_SpeedReading = 0; }
    if (TurnRight_SpeedReading < 16) { TurnRight_SpeedReading = 0; }
    Stationary();
  }
  else
  { 
    Forward_SpeedReading -= 15;
    Backward_SpeedReading -= 15;
    TurnLeft_SpeedReading -= 20;
    TurnRight_SpeedReading -= 20;
    if (Forward_SpeedReading < 16) { Forward_SpeedReading = 0; }
    if (Backward_SpeedReading < 16) { Backward_SpeedReading = 0; }
    if (TurnLeft_SpeedReading < 16) { TurnLeft_SpeedReading = 0; }
    if (TurnRight_SpeedReading < 16) { TurnRight_SpeedReading = 0; }
    Stationary();
  }

  // if (linear_x < 0.2) { Stationary(); }
}

ros::Subscriber <geometry_msgs::Twist> sub("/cmd_vel", &CallBackFunction);

void setup() {
  // ROS
  nh.initNode();                    // Initiate the node
  nh.subscribe(sub);                // Subscribe to topic
  nh.advertise(pub);

  toggleSwitch.setDebounceTime(50); // Debounce time of 50 milliseconds

  // 2-Axis Joystick
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);

  // MDD10A Rev2.0 Motor Driver #1
  pinMode(MotorDriver1_Direction1, OUTPUT);
  pinMode(MotorDriver1_Speed1, OUTPUT);
  pinMode(MotorDriver1_Direction2, OUTPUT);
  pinMode(MotorDriver1_Speed2, OUTPUT);
  analogWrite(MotorDriver1_Speed1, 0);
  analogWrite(MotorDriver1_Speed2, 0);
  
  // MDD10A Rev2.0 Motor Driver #2
  pinMode(MotorDriver2_Direction1, OUTPUT);
  pinMode(MotorDriver2_Speed1, OUTPUT);
  pinMode(MotorDriver2_Direction2, OUTPUT);
  pinMode(MotorDriver2_Speed2, OUTPUT);
  analogWrite(MotorDriver2_Speed1, 0);
  analogWrite(MotorDriver2_Speed2, 0);

  // Servo Motor
  servoMotor.attach(servo_pin);
  servoMotor.write(0);
  delay(20);

  Serial.begin(57600);
  nh.getHardware() -> setBaud(57600);
  nh.initNode();
}

void loop() {
  // ON/OFF Switch condition
  toggleSwitch.loop();
  int state = toggleSwitch.getState();
  if (state == 0)
  {
    // Serial.println("AUTO");
    nh.spinOnce();
    servoMotor.write(0);
    // float angular_z = 0.01;
    // int angular_zCONVERTED = abs(int(round(angular_z * 10000)));
    // int arduino_value = map(angular_zCONVERTED, 0, 5672, 0, 255);
    // Serial.println(arduino_value);
    // FakeAuto();
  }
  else if (state == 1)
  {
    // Serial.println("MANUAL");
    servoMotor.write(170);
    xAxis_Reading = analogRead(X_pin);
    yAxis_Reading = analogRead(Y_pin);
    // Serial.print("yAXIS: ");
    // Serial.print(yAxis_Reading);
    // Serial.print(", xAXIS: ");
    // Serial.print(xAxis_Reading);
    // Serial.println("\n");

    Serial.println(Forward_SpeedReading);

    // int angular_z_speed = abs(int(round(angular_z * 10000)));
    // angular_z_speed = map(angular_z_speed, 0, 5672, 30, 175);
    // int yAxis_speed_forward = map(yAxis_Reading, 300, 6, 20, 90);


    if ((yAxis_Reading < 100) && (xAxis_Reading > 25 && xAxis_Reading < 1000)) 
    { 
      Forward(0, Forward_SpeedReading, state);
      Backward_SpeedReading = TurnLeft_SpeedReading = TurnRight_SpeedReading = 0;
    }
    else if ((yAxis_Reading > 900) && (xAxis_Reading > 25 && xAxis_Reading < 1000)) 
    { 
      Backward(0, Backward_SpeedReading, state);
      Forward_SpeedReading = TurnLeft_SpeedReading = TurnRight_SpeedReading = 0;
    }
    else if ((xAxis_Reading < 100) && (yAxis_Reading > 25 && yAxis_Reading < 1000)) 
    { 
      TurnLeft(0, TurnLeft_SpeedReading, state);
      Forward_SpeedReading = Backward_SpeedReading = TurnRight_SpeedReading = 0;
    }
    else if ((xAxis_Reading > 900) && (yAxis_Reading > 25 && yAxis_Reading < 1000)) 
    { 
      TurnRight(0, TurnRight_SpeedReading, state);
      Forward_SpeedReading = Backward_SpeedReading = TurnLeft_SpeedReading = 0;
    }
    // else if ((yAxis_Reading > 900) && (xAxis_Reading < 500)) 
    // { 
    //   RealStationary();
    // }
    else
    { 
      Forward_SpeedReading -= 15;
      Backward_SpeedReading -= 15;
      TurnLeft_SpeedReading -= 20;
      TurnRight_SpeedReading -= 20;
      if (Forward_SpeedReading < 16) { Forward_SpeedReading = 0; }
      if (Backward_SpeedReading < 16) { Backward_SpeedReading = 0; }
      if (TurnLeft_SpeedReading < 16) { TurnLeft_SpeedReading = 0; }
      if (TurnRight_SpeedReading < 16) { TurnRight_SpeedReading = 0; }
      Stationary();
    }

    // delay(100);
  }
  delay(100);
  nh.spinOnce();
}

void Forward(int autoSpeed, int& manualSpeed, int state){
  currentMovement = "Forward";

  int speed;
  if (state == 1)
  {
    if (manualSpeed+5 <= 90) { manualSpeed += 5; }
    speed = manualSpeed;
  }
  else if (state == 0) 
  { 
    if (manualSpeed+5 <= autoSpeed) { manualSpeed += 5; }
    speed = manualSpeed;
  }

  // MDD10A Rev2.0 Motor Driver #1
  digitalWrite(MotorDriver1_Direction1, HIGH);
  digitalWrite(MotorDriver1_Direction2, LOW);
  analogWrite(MotorDriver1_Speed1, speed);
  analogWrite(MotorDriver1_Speed2, speed);
  
  // MDD10A Rev2.0 Motor Driver #2
  digitalWrite(MotorDriver2_Direction1, HIGH);
  digitalWrite(MotorDriver2_Direction2, LOW);
  analogWrite(MotorDriver2_Speed1, speed);
  analogWrite(MotorDriver2_Speed2, speed);
}

void Backward(int autoSpeed, int& manualSpeed, int state){
  currentMovement = "Backward";

  int speed;
  if (state == 1)
  {
    if (manualSpeed+5 <= 90) { manualSpeed += 5; }
    speed = manualSpeed;
  }
  else if (state == 0) 
  { 
    if (manualSpeed+5 <= autoSpeed) { manualSpeed += 5; }
    speed = manualSpeed;
  }

  // MDD10A Rev2.0 Motor Driver #1
  digitalWrite(MotorDriver1_Direction1, LOW);
  digitalWrite(MotorDriver1_Direction2, HIGH);
  analogWrite(MotorDriver1_Speed1, speed);
  analogWrite(MotorDriver1_Speed2, speed);

  // MDD10A Rev2.0 Motor Driver #2
  digitalWrite(MotorDriver2_Direction1, LOW);
  digitalWrite(MotorDriver2_Direction2, HIGH);
  analogWrite(MotorDriver2_Speed1, speed);
  analogWrite(MotorDriver2_Speed2, speed);
}

void TurnLeft(int autoSpeed, int& manualSpeed, int state){
  currentMovement = "Left";

  int speed;
  if (state == 1)
  {
    if (manualSpeed+5 <= 90) { manualSpeed += 5; }
    speed = manualSpeed;
  }
  else if (state == 0) 
  { 
    if (manualSpeed+5 <= autoSpeed) { manualSpeed += 5; }
    speed = manualSpeed;
  }

  // MDD10A Rev2.0 Motor Driver #1
  digitalWrite(MotorDriver1_Direction1, HIGH);
  digitalWrite(MotorDriver1_Direction2, LOW);
  analogWrite(MotorDriver1_Speed1, speed);
  analogWrite(MotorDriver1_Speed2, speed);

  // MDD10A Rev2.0 Motor Driver #2
  digitalWrite(MotorDriver2_Direction1, LOW);
  digitalWrite(MotorDriver2_Direction2, HIGH);
  analogWrite(MotorDriver2_Speed1, speed);
  analogWrite(MotorDriver2_Speed2, speed);
}

void TurnRight(int autoSpeed, int& manualSpeed, int state){
  currentMovement = "Right";

  int speed;
  if (state == 1)
  {
    if (manualSpeed+5 <= 90) { manualSpeed += 5; }
    speed = manualSpeed;
  }
  else if (state == 0) 
  { 
    if (manualSpeed+5 <= autoSpeed) { manualSpeed += 5; }
    speed = manualSpeed;
  }

  // MDD10A Rev2.0 Motor Driver #1
  digitalWrite(MotorDriver1_Direction1, LOW);
  digitalWrite(MotorDriver1_Direction2, HIGH);
  analogWrite(MotorDriver1_Speed1, speed);
  analogWrite(MotorDriver1_Speed2, speed);

  // MDD10A Rev2.0 Motor Driver #2
  digitalWrite(MotorDriver2_Direction1, HIGH);
  digitalWrite(MotorDriver2_Direction2, LOW);
  analogWrite(MotorDriver2_Speed1, speed);
  analogWrite(MotorDriver2_Speed2, speed);
}

void Stationary(){
  int speed = 0;
  if (currentMovement == "Forward")
  { 
    digitalWrite(MotorDriver1_Direction1, HIGH);
    digitalWrite(MotorDriver1_Direction2, LOW);
    digitalWrite(MotorDriver2_Direction1, HIGH);
    digitalWrite(MotorDriver2_Direction2, LOW);
    speed = Forward_SpeedReading;
  }
  else if (currentMovement == "Backward")
  { 
    digitalWrite(MotorDriver1_Direction1, LOW);
    digitalWrite(MotorDriver1_Direction2, HIGH);
    digitalWrite(MotorDriver2_Direction1, LOW);
    digitalWrite(MotorDriver2_Direction2, HIGH);
    speed = Backward_SpeedReading;
  }
  else if (currentMovement == "Left")
  { 
    digitalWrite(MotorDriver1_Direction1, HIGH);
    digitalWrite(MotorDriver1_Direction2, LOW);
    digitalWrite(MotorDriver2_Direction1, LOW);
    digitalWrite(MotorDriver2_Direction2, HIGH);
    speed = TurnLeft_SpeedReading;
  }
  else if (currentMovement == "Right")
  { 
    digitalWrite(MotorDriver1_Direction1, LOW);
    digitalWrite(MotorDriver1_Direction2, HIGH);
    digitalWrite(MotorDriver2_Direction1, HIGH);
    digitalWrite(MotorDriver2_Direction2, LOW);
    speed = TurnRight_SpeedReading;
  }

  if (currentMovement == "Stationary" || speed == 0) 
  { 
    digitalWrite(MotorDriver1_Direction1, LOW);
    digitalWrite(MotorDriver1_Direction2, LOW);
    digitalWrite(MotorDriver2_Direction1, LOW);
    digitalWrite(MotorDriver2_Direction2, LOW);
    currentMovement = "Stationary";
  }

  // MDD10A Rev2.0 Motor Driver #1
  // digitalWrite(MotorDriver1_Direction1, LOW);
  // digitalWrite(MotorDriver1_Direction2, LOW);
  analogWrite(MotorDriver1_Speed1, speed);
  analogWrite(MotorDriver1_Speed2, speed);

  // MDD10A Rev2.0 Motor Driver #2
  // digitalWrite(MotorDriver2_Direction1, LOW);
  // digitalWrite(MotorDriver2_Direction2, LOW);
  analogWrite(MotorDriver2_Speed1, speed);
  analogWrite(MotorDriver2_Speed2, speed);
}

/*void RealStationary(){
  // MDD10A Rev2.0 Motor Driver #1
  digitalWrite(MotorDriver1_Direction1, LOW);
  digitalWrite(MotorDriver1_Direction2, LOW);
  analogWrite(MotorDriver1_Speed1, 0);
  analogWrite(MotorDriver1_Speed2, 0);

  // MDD10A Rev2.0 Motor Driver #2
  digitalWrite(MotorDriver2_Direction1, LOW);
  digitalWrite(MotorDriver2_Direction2, LOW);
  analogWrite(MotorDriver2_Speed1, 0);
  analogWrite(MotorDriver2_Speed2, 0);
}*/


/*void FakeAuto(){
  // MDD10A Rev2.0 Motor Driver #1
  digitalWrite(MotorDriver1_Direction1, LOW);
  digitalWrite(MotorDriver1_Direction2, HIGH);
  analogWrite(MotorDriver1_Speed1, 25);
  analogWrite(MotorDriver1_Speed2, 25);

  // MDD10A Rev2.0 Motor Driver #2
  digitalWrite(MotorDriver2_Direction1, HIGH);
  digitalWrite(MotorDriver2_Direction2, LOW);
  analogWrite(MotorDriver2_Speed1, 25);
  analogWrite(MotorDriver2_Speed2, 25);
}*/




