const int freq = 5000;
const int resolution = 256;

const int motorAPin1 = 16;
const int motorAPin2 = 17;
const int motorBPin1 = 18;
const int motorBPin2 = 19;

const int motorAChannel1 = 0;
const int motorAChannel2 = 1;
const int motorBChannel1 = 2;
const int motorBChannel2 = 3;

void setup() {
  // put your setup code here, to run once:
  ledcSetup(motorAChannel1, freq, resolution);
  ledcSetup(motorAChannel2, freq, resolution);
  ledcSetup(motorBChannel1, freq, resolution);
  ledcSetup(motorBChannel2, freq, resolution);
  ledcAttachPin(motorAPin1, motorAChannel1);
  ledcAttachPin(motorAPin2, motorAChannel2);
  ledcAttachPin(motorBPin1, motorBChannel1);
  ledcAttachPin(motorBPin2, motorBChannel2);
}

void loop() {
  // put your main code here, to run repeatedly:
  ledcWrite(motorAChannel1, 150);
  ledcWrite(motorAChannel2, 0);
  ledcWrite(motorBChannel1, 150);
  ledcWrite(motorBChannel2, 0);
  delay(1000);
  ledcWrite(motorAChannel1, 0);
  ledcWrite(motorAChannel2, 0);
  ledcWrite(motorBChannel1, 0);
  ledcWrite(motorBChannel2, 0);
  delay(1000);
  ledcWrite(motorAChannel1, 0);
  ledcWrite(motorAChannel2, 150);
  ledcWrite(motorBChannel1, 0);
  ledcWrite(motorBChannel2, 150);
    delay(1000);
  ledcWrite(motorAChannel1, 0);
  ledcWrite(motorAChannel2, 0);
  ledcWrite(motorBChannel1, 0);
  ledcWrite(motorBChannel2, 0);
  delay(1000);
}
