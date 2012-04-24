
#include <Servo.h>
#include <avr/eeprom.h>

#define D3 3
#define D4 4 // button
#define D5 5
#define D7 7
#define D8 8
#define D9 9
#define D11 11
#define D12 12
#define D13 13

#define EE_Settings 0

inline void *operator new(size_t, void *p) { return p; }

struct Settings {
  Settings() {
    sCenterSteer = 90;
    sSideSteer = 20;
    sFullSpeed = 255;
    sHalfSpeed = 100;
    sPollInterval = 100;
    sRangeStop = 100;
  }

  int sig;
  int sCenterSteer;
  int sSideSteer;
  int sFullSpeed;
  int sHalfSpeed;
  int sPollInterval;
  int sRangeStop;

  void visit(void (*func)(char, int *, char const *)) {
    (*func)('C', &sCenterSteer, "CenterSteer");
    (*func)('S', &sSideSteer, "SideSteer");
    (*func)('F', &sFullSpeed, "FullSpeed");
    (*func)('H', &sHalfSpeed, "HalfSpeed");
    (*func)('P', &sPollInterval, "PollInterval");
    (*func)('R', &sRangeStop, "RangeStop");
  }
  int calcSig() {
    return 
      (sCenterSteer << 1) ^
      (sSideSteer << 2) ^
      (sFullSpeed << 3) ^
      (sHalfSpeed << 4) ^
      (sPollInterval << 5) ^
      (sRangeStop << 6);
  }
  bool load() {
    eeprom_read_block(this, (char const *)EE_Settings, sizeof(*this));
    return calcSig() == this->sig;
  }
  void save() {
    sig = calcSig();
    eeprom_write_block(this, (char *)EE_Settings, sizeof(*this));
  }
};
Settings stg;

#define CenterSteer stg.sCenterSteer

#define SideSteer stg.sSideSteer
#define FullSpeed stg.sFullSpeed
#define HalfSpeed stg.sHalfSpeed
#define PollInterval stg.sPollInterval
#define RangeStop stg.sRangeStop

class MotorControl {
  public:
    MotorControl(int dirPin, int pwmPin, int brakePin, int currentPin) :
      dirPin_(dirPin),
      pwmPin_(pwmPin),
      brakePin_(brakePin),
      currentPin_(currentPin)
    {
    }
    void reset()
    {
      pinMode(dirPin_, OUTPUT);
      digitalWrite(dirPin_, HIGH);
      pinMode(pwmPin_, OUTPUT);
      analogWrite(pwmPin_, 0);
      pinMode(brakePin_, OUTPUT);
      digitalWrite(brakePin_, HIGH);
    }
    int dirPin_;
    int pwmPin_;
    int brakePin_;
    int currentPin_;
    void brake() {
      analogWrite(pwmPin_, 0);
      digitalWrite(dirPin_, HIGH);
      digitalWrite(brakePin_, HIGH);
    }
    void freeWheel() {
      digitalWrite(brakePin_, LOW);
      analogWrite(pwmPin_, 0);
      digitalWrite(dirPin_, HIGH);
    }
    void go(int pwrDir = 255) {
      digitalWrite(brakePin_, LOW);
      if (pwrDir > 255) {
        pwrDir = 255;
      }
      else if (pwrDir < -255) {
        pwrDir = -255;
      }
      if (pwrDir < 0) {
        digitalWrite(dirPin_, LOW);
        pwrDir = -pwrDir;
      }
      else {
        digitalWrite(dirPin_, HIGH);
      }
      analogWrite(pwmPin_, pwrDir);
    }
    int readMilliAmps() {
      int rd = analogRead(currentPin_);
      return rd * 3;  //  2 amps at 3.3V 
    }
};

MotorControl mcFront(D12, D3, D9, A0);
MotorControl mcBack(D13, D11, D8, A1);
Servo sSteer;

enum StateId {
  Stopped = 0,
  SlowForward = 1,
  FastForward = 2,
  Coasting = 3,
  SlowBackward = 4,
  Paused = 5,
};

bool button = false;
int state = Stopped;
unsigned long nextStateAt = 0;

unsigned long nextMeasureAt = 0;

char cmdLine[32];
unsigned char clPtr = 0;



void eStop()
{
    state = 0;
    nextStateAt = 0;
    mcFront.brake();
    mcBack.brake();
    clPtr = 0;
    Serial.println("e!");
}

void clampParams()
{
  if (CenterSteer - SideSteer < 0) {
    CenterSteer = 90;
  }
  if (CenterSteer + SideSteer > 180) {
    CenterSteer = 90;
  }
  if (SideSteer > 60 || SideSteer < -60) {
    SideSteer = 20;
  }
  if (FullSpeed < 0 || FullSpeed > 255) {
    FullSpeed = 255;
  }
  if (HalfSpeed < 0 || HalfSpeed > 255) {
    HalfSpeed = 100;
  }
  if (PollInterval > 1000 || PollInterval < 1) {
    PollInterval = 100;
  }
  if (RangeStop < 10) {
    RangeStop = 10;
  }
}

void vPrint(char ch, int *val, char const *str)
{
  char buf[2] = { ch, 0 };
  Serial.print(buf);
  Serial.print(*val);
  Serial.print(" ");
  Serial.println(str);
}

void printParams()
{
  stg.visit(&vPrint);
}

void writeParams()
{
  stg.save();
}

void readParams()
{
  if (!stg.load()) {
    new (&stg) Settings();
  }
}

bool cmdOk;

void vCmdLine(char ch, int *p, char const *
)
{
  if (cmdLine[0] == ch) {
    *p = atoi(&cmdLine[1]);
    cmdOk = true;
  }
}

void doCmdLine()
{
  //  Adjust steering
  cmdOk = false;
  if (cmdLine[0] == '?') {
    cmdOk = true;
  }
  stg.visit(&vCmdLine);
  if (!cmdOk) {
    Serial.println("ERROR. Use ? for params.");
    return;
  }
  clampParams();
  writeParams();
  printParams();
  Serial.println("OK");
}

void pollCmdLine()
{
  for (int n = Serial.available(); n != 0; --n) {
    char ch = Serial.read();
    if (ch == 13) {
      if (clPtr < sizeof(cmdLine)-1) {
        doCmdLine();
      }
      else {
        Serial.println("ERROR. Too long cmd.");
      }
      clPtr = 0;
      cmdLine[0] = 0;
    }
    else if (ch == 10 || ch == 8 || ch == 32) {
      //  do nothing
    }
    else if (ch == 8) {
      if (clPtr > 0) {
        clPtr--;
      }
    }
    else if (clPtr < sizeof(cmdLine)-1) {
      cmdLine[clPtr++] = ch;
      cmdLine[clPtr] = 0;
    }
  }
}

void setup()
{
  mcFront.reset();
  mcBack.reset();
  pinMode(D4, INPUT);
  digitalWrite(D4, HIGH);
  sSteer.attach(D5);
  Serial.begin(57600);
  Serial.println(__DATE__ " " __TIME__);
  readParams();
  clampParams();
  printParams();
}

void loop()
{
  unsigned long mill = millis();
  
  if (digitalRead(D4) == LOW) {
    if (!button) {
      button = true;
      if (state == 0) {
        nextStateAt = mill | 1;
        nextMeasureAt = nextStateAt + 1;
      }
      else {
        eStop();
      }
      Serial.println("b1");
    }
  }
  else if (button) {
    delay(50);  //  de-bounce
    button = false;
  }
  
  switch (state) {
    default:
      /* weird state? */
      state = 0;
      nextStateAt = 0;
      
    case Stopped:
      mcFront.brake();
      mcBack.brake();
      sSteer.write(CenterSteer);
      pollCmdLine();
      break;
    case SlowForward:
      mcFront.go(HalfSpeed);
      mcBack.go(HalfSpeed);
      sSteer.write(CenterSteer + SideSteer);
      break;
    case FastForward:
      mcFront.go(FullSpeed);
      mcBack.go(FullSpeed);
      sSteer.write(CenterSteer);
      break;
    case Coasting:
      mcFront.freeWheel();
      mcBack.freeWheel();
      sSteer.write(CenterSteer);
      break;
    case SlowBackward:
      mcFront.go(-HalfSpeed);
      mcBack.go(-HalfSpeed);
      sSteer.write(CenterSteer - SideSteer);
      break;
    case Paused:
      mcFront.brake();
      mcBack.brake();
      sSteer.write(CenterSteer);
      break;
  }
  static int lastState = 0;
  if (state != lastState) {
    Serial.print(" -> ");
    Serial.println(state);
    lastState = state;
  }
  
  
  
  if (nextStateAt != 0) {
    if ((long)(mill - nextStateAt) >= 0) {
      state = state + 1;
      nextStateAt = mill + 2000;
    }
  }
  
  if ((long)(mill - nextMeasureAt) >= 0) {
    static int nMeasure = 0;
    if (++nMeasure == 64) {
      Serial.println(".");
      nMeasure = 0;
    }
    else {
      Serial.print(".");
    }
    pinMode(D7, OUTPUT);
    digitalWrite(D7, LOW);
    delayMicroseconds(2);
    digitalWrite(D7, HIGH);
    delayMicroseconds(4);

    digitalWrite(D7, LOW);
    pinMode(D7, INPUT);
    unsigned long time = pulseIn(D7, HIGH);
    unsigned long cm = time / 58;
    if (cm < RangeStop) {
      if (state != 0) {
        eStop();
      }
      
      Serial.println(cm);
    }
    nextMeasureAt = mill + ((state == 0) ? 25 * PollInterval : PollInterval);
  }
}

