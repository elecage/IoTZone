#include <WiFi.h>

// NTP settings
#define TIMEZONE 9 // timezone (GMT = 0, Japan = 9)
#define NTP_SERVER "pool.ntp.org"

#define WIFI_SMARTCONFIG true

#if !WIFI_SMARTCONFIG
// if you do not use smartConfifg, please specify SSID and password here
#define WIFI_SSID "SSID" // your WiFi's SSID
#define WIFI_PASS "PASS" // your WiFi's password
#endif

//////////////////////////////
// motor control section
//////////////////////////////

// Motor parameters
#define STEPS_PER_ROTATION 2048 // steps of a single rotation of motor
#define POSITION 60 // each wheel has 9 positions
#define MOTOR_DELAY 2400
#define INITIAL_POS 10

#define PHASE 4
#define PINS 4
// ports used to control the stepper motor.
// if your motor rotates to the opposite direction, change to
//int port[PINS] = {12, 4, 0, 2};
int port[PINS] = {2, 0, 4, 12};

// sequence of stepper motor control
int seq[PHASE][PINS] = {
  {  LOW,  LOW, HIGH,  LOW},
  {  LOW,  LOW,  LOW, HIGH},
  { HIGH,  LOW,  LOW,  LOW},
  {  LOW, HIGH,  LOW,  LOW}
};

// stepper motor rotation
void rotate(int step) {
  static int phase = 0;
  int i, j;
  int delta = (step > 0) ? 1 : (PHASE-1);
  int contmode = 0;
  
  if(abs(step) <= STEPS_PER_ROTATION / POSITION + 1) { // 1 sec advance
    contmode = 1;
  }
  
  step = (step > 0) ? step : -step;
  for(j = 0; j < step; j++) {
    phase = (phase + delta) % PHASE;
    for(i = 0; i < PINS; i++) {
      digitalWrite(port[i], seq[phase][i]);
    }
    if(contmode) {
      delayMicroseconds(1000000L / (STEPS_PER_ROTATION / POSITION + 1));
    }
    else {
      delayMicroseconds(MOTOR_DELAY);
      if(j < 20) delayMicroseconds(MOTOR_DELAY); // acceleration
      if(j < 10) delayMicroseconds(MOTOR_DELAY); // acceleration
      if(step - j < 10) delayMicroseconds(MOTOR_DELAY); // deacceleration
      if(step - j < 5) delayMicroseconds(MOTOR_DELAY); // deacceleration
      if(step - j < 2) delayMicroseconds(MOTOR_DELAY); // deacceleration
    }
  }
  // power cut
  for(i = 0; i < PINS; i++) {
    digitalWrite(port[i], LOW);
  }
}

//////////////////////////////
// rorary clock control section
//////////////////////////////

#define DIGIT 3
typedef struct {
  int v[DIGIT];
} Digit;

void printDigit(Digit d);
Digit rotUp(Digit current, int digit, int num);
Digit rotDown(Digit current, int digit, int num);
Digit rotDigit(Digit current, int digit, int num);
Digit setDigit(Digit current, int digit, int num);
int setNumber(Digit n);

// avoid error accumuration of fractional part of 4096 / POSITION
void rotStep(int s) {
  static long currentPos;
  static long currentStep;
  
  currentPos += s;
  long diff = currentPos * STEPS_PER_ROTATION / POSITION - currentStep;
  rotate(diff);
  currentStep += diff;
}

void printDigit(Digit d) {
  String s = "            ";
  int i;

  for(i = 0; i < DIGIT; i++) {
    s.setCharAt(i, d.v[i] + '0');
  }
  Serial.println(s);
}

Digit current = {0};

//increase specified digit
Digit rotUp(Digit current, int digit, int num) {
  int freeplay = 0;
  int i;
  
  for(i = digit; i < DIGIT - 1; i++) {
    int id = current.v[i];
    int nd = current.v[i+1];
    if(id <= nd) id += POSITION;
    freeplay += id - nd - 1;
  }
  freeplay += num;
  rotStep(-1 * freeplay);
  current.v[digit] = (current.v[digit] + num) % POSITION;
  for(i = digit + 1; i < DIGIT; i++) {
    current.v[i] = (current.v[i - 1] + (POSITION - 1)) % POSITION;
  }

  Serial.print("up end : ");
  printDigit(current);
  return current;
}

// decrease specified digit
Digit rotDown(Digit current, int digit, int num) {
  int freeplay = 0;
  int i;
  
  for(i = digit; i < DIGIT - 1; i++) {
    int id = current.v[i];
    int nd = current.v[i+1];
    if(id > nd) nd += POSITION;
    freeplay += nd - id;
  }
  freeplay += num;
  rotStep( 1 * freeplay);
  current.v[digit] = (current.v[digit] - num + POSITION) % POSITION;
  for(i = digit + 1; i < DIGIT; i++) {
    current.v[i] = current.v[i - 1];
  }

  Serial.print("down end : ");
  printDigit(current);
  return current;
}

// decrease or increase specified digit 
Digit rotDigit(Digit current, int digit, int num) {
  if(num > 0) {
    return rotUp(current, digit, num);
  }
  else if(num < 0) {
    return rotDown(current, digit, -num);
  }
  else return current;
}

// set single digit to the specified number
Digit setDigit(Digit current, int digit, int num) {
  int pd, cd = current.v[digit];
  if(digit == 0) { // most significant digit
    pd = 0;
  }
  else {
    pd = current.v[digit - 1];
  }
  if(cd == num) return current;
  
  // check if increasing rotation is possible
  int n2 = num;
  if(n2 < cd) n2 += POSITION;
  if(pd < cd) pd += POSITION;
  if(pd <= cd || pd > n2) {
    return rotDigit(current, digit, n2 - cd);
  }
  // if not, do decrease rotation
  if(num > cd) cd += POSITION;
  return rotDigit(current, digit, num - cd);  
}

int setNumber(Digit n) {
  int i;
  
  for(i = 0; i < DIGIT; i++) {
    if(current.v[i] != n.v[i]) {
      break;
    }
  }
  // rotate most significant wheel only to follow current time ASAP
  if(i < DIGIT) {
      current = setDigit(current, i, n.v[i]);
  }
  if(i >= DIGIT - 1) {
    return true; // finished
  }
  else {
    return false;
  }
}

//////////////////////////////
// WiFi and NTP section
//////////////////////////////

void getNTP(void) {
  for(int i = 0; WiFi.status() != WL_CONNECTED; i++) {
    if(i > 30) {
      ESP.restart();
    }
    Serial.println("Waiting for WiFi connection..");
    delay(1000);
  }

  configTime(TIMEZONE * 3600L, 0, NTP_SERVER);
  printLocalTime();
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %Y-%m-%d %H:%M:%S");
}

void wifiSetup() {
  int wifiMotion = STEPS_PER_ROTATION / 10; // while wainting for wifi, large motion
  int smatconfigMotion = STEPS_PER_ROTATION / 50; // while wainting for smartConfig, small motion
  int smartconfigCount = 0;

  WiFi.mode(WIFI_STA);
#if WIFI_SMARTCONFIG
  WiFi.begin();
#else
  WiFi.begin(WIFI_SSID, WIFI_PASS);
#endif

  for (int i = 0; ; i++) {
    Serial.println("Connecting to WiFi...");
    rotate(wifiMotion); wifiMotion *= -1;
    delay(1000);
    if (WiFi.status() == WL_CONNECTED) {
      break;
    }
#if WIFI_SMARTCONFIG
  if(i > 6)
    break;
#endif    
  }

#if WIFI_SMARTCONFIG
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.mode(WIFI_AP_STA);
    WiFi.beginSmartConfig();

    //Wait for SmartConfig packet from mobile
    Serial.println("Waiting for SmartConfig.");
    while (!WiFi.smartConfigDone()) {
      Serial.print(".");
      rotate(smatconfigMotion); smatconfigMotion *= -1;
      delay(1000);
      smartconfigCount++;
      if(smartconfigCount > 600) { // wait for 10 min, then reset
        ESP.restart();
      }
    }

    Serial.println("");
    Serial.println("SmartConfig received.");

    //Wait for WiFi to connect to AP
    Serial.println("Waiting for WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      rotate(wifiMotion); wifiMotion *= -1;
      delay(1000);
      Serial.print(",");
    }
  }
  Serial.println("WiFi Connected.");
#endif

  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

//////////////////////////////
// setup and main loop
//////////////////////////////

void setup() {
  Serial.begin(115200);
  Serial.println("start");

  pinMode(port[0], OUTPUT);
  pinMode(port[1], OUTPUT);
  pinMode(port[2], OUTPUT);
  pinMode(port[3], OUTPUT);

  wifiSetup();
  
  rotate(STEPS_PER_ROTATION * DIGIT); // reset all digits using physical end stop

  rotate(-INITIAL_POS);
  
  getNTP(); // get current time
}

void loop() {
  static int prevhour = -1;
  struct tm tmtime;
  boolean finished;
  Digit n;

  getLocalTime(&tmtime);
  
  if(tmtime.tm_hour == 23 && tmtime.tm_min == 59 && tmtime.tm_sec >= 58)
    ESP.restart(); // reset everyday
  if(tmtime.tm_min >= 55 && tmtime.tm_sec >= tmtime.tm_min)
    return; // just wait for sec == 0

  tmtime.tm_hour %= 12;
  n.v[0] = tmtime.tm_hour * 5 + tmtime.tm_min / 12;
  n.v[1] = tmtime.tm_min;
  n.v[2] = tmtime.tm_sec;
  if(n.v[1] > current.v[2] && current.v[2] > n.v[2]) {
    n.v[2] = n.v[2] + (current.v[2] - n.v[2]) / 15;
  }
  if(n.v[1] == n.v[2]) { // min and sec hands interfere
    n.v[2] = (n.v[2] + 4) % POSITION; // it takes around 5 sec for full turn
  }
  
  finished = setNumber(n);

  if(finished && tmtime.tm_hour != prevhour) {
    if(tmtime.tm_hour % 6 == 0)
      getNTP();
    prevhour = tmtime.tm_hour;
  }
}
