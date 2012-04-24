#include <Servo.h>

unsigned char penUpPos = 35;
unsigned char penDownPos = 90;
int loopDelay = 700;
bool a0state = true;

void ctl(int phase, int pin) {
  unsigned char foo = 0;
  switch (phase & 7) {
    case 0: foo = 1; break;
    case 1: foo = 3; break;
    case 2: foo = 2; break;
    case 3: foo = 6; break;
    case 4: foo = 4; break;
    case 5: foo = 12; break;
    case 6: foo = 8; break;
    case 7: foo = 9; break;
  }
  for (int i = 0; i < 4; ++i) {
    if (foo & (1 << i)) {
      digitalWrite(pin + i, HIGH);
    }
    else {
      digitalWrite(pin + i, LOW);
    }
  }
}

class XYTarget {
public:
  XYTarget(int xp, int yp, int de) :
    xpin(xp),
    ypin(yp),
    delay(de)
  {
    reset();
  }
  void reset()
  {
    xenable = true;
    yenable = true;
    xpos = 0;
    ypos = 0;
    xphase = 0;
    yphase = 0;
    phaseTarget = 0;
    xphaseInc = 0;
    yphaseInc = 0;
    xdest = 0;
    ydest = 0;
    delayCnt = 0;
    drivePins();
  }
  int xpin;
  int ypin;
  int xpos;
  int ypos;
  int xphase;
  int yphase;
  int phaseTarget;
  int xphaseInc;
  int yphaseInc;
  int xdest;
  int ydest;
  int delay;
  int delayCnt;
  bool xenable;
  bool yenable;
  
  void setTarget(int x, int y)
  {
    xdest = x;
    ydest = y;
    xphase = 0;
    yphase = 0;
    xphaseInc = abs(xdest - xpos);
    yphaseInc = abs(ydest - ypos);
    if (xphase > yphase) {
      phaseTarget = xphaseInc;
    }
    else {
      phaseTarget = yphaseInc;
    }
  }

  void enableMotors(bool a, bool b) {
    xenable = a;
    yenable = b;
    drivePins();
  }
  
  void disablePins(int pin) {
    for (int i = 0; i < 4; ++i) {
      digitalWrite(i + pin, LOW);
    }
  }
  
  void drivePins() {
    if (xenable) {
      ctl(xpos, xpin);
    }
    else {
      disablePins(xpin);
    }
    if (yenable) {
      ctl(ypos, ypin);
    }
    else {
      disablePins(ypin);
    }
  }

  void setDelay(int de) {
    delay = de;
  }  
  
  //  returns true if found the target position
  bool step()
  {
    if ((xpos == xdest) && (ypos == ydest)) {
      drivePins();
      return true;
    }
    if (delayCnt > 0) {
      --delayCnt;
      return false;
    }
    delayCnt = delay;
    xphase += xphaseInc;
    if (xphase >= phaseTarget) {
      xphase -= phaseTarget;
      if (xdest > xpos) {
        ++xpos;
      }
      else if (xdest < xpos) {
        --xpos;
      }
    }
    yphase += yphaseInc;
    if (yphase >= phaseTarget) {
      yphase -= phaseTarget;
      if (ydest > ypos) {
        ++ypos;
      }
      else if (ydest < ypos) {
        --ypos;
      }
    }
    drivePins();
    return false;
  }
};

XYTarget target(2, 6, 4);
Servo s;
bool penIsUp = true;

int stepsToSkip = 0;

char curCmd[33];
char nextCmd[33];
int nextPtr = 0;

void printVersion()
{
  Serial.write("EBB ovalbot " __DATE__ "\r\n");
}

void setup()
{
  Serial.begin(9600);

  for (int i = 2; i < 14; ++i)
  {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
  pinMode(A0, INPUT);
  digitalWrite(A0, HIGH);  //  pull-up
  digitalWrite(13, HIGH);
  s.attach(10);
  s.write(penUpPos-15);
  delay(1500);
  s.write(penUpPos);
  target.step();
}

void penUp()
{
  digitalWrite(13, HIGH);
  s.write(penUpPos-15);
  penIsUp = true;
  delay(1500);
  s.write(penUpPos);
}

void penDown()
{
  digitalWrite(13, LOW);
  s.write(penDownPos);
  penIsUp = false;
  delay(500);
}

void reset()
{
  penUp();
  target.setTarget(0, 0);
  loopDelay = 700;
  stepsToSkip = 0;
}

void abortError(char const *msg) {
  Serial.print(msg);
  Serial.print("\r\n");
  reset();
  nextPtr = 0;
}

void doCommand()
{
  if (curCmd[0] == 'v') {
    //  version
    printVersion();
    return;
  }
  if (curCmd[0] == 'Q' && curCmd[1] == 'P') {
    //  query pen
    if (penIsUp) {
      Serial.write("1\r\n");
    }
    else {
      Serial.write("0\r\n");
    }
    return;
  }
  if (curCmd[0] == 'Q' && curCmd[1] == 'B') {
    //  query button -- I have no button right now
    Serial.write("0\r\n");
    return;
  }
  if (curCmd[0] == 'Q' && curCmd[1] == 'A') {
    Serial.write("xpin "); Serial.print(target.xpin); Serial.write(", ");
    Serial.write("ypin "); Serial.print(target.ypin); Serial.write(", ");
    Serial.write("xpos "); Serial.print(target.xpos); Serial.write(", ");
    Serial.write("ypos "); Serial.print(target.ypos); Serial.write(", ");
    Serial.write("xphase "); Serial.print(target.xphase); Serial.write(", ");
    Serial.write("yphase "); Serial.print(target.yphase); Serial.write(", ");
    Serial.write("phaseTarget "); Serial.print(target.phaseTarget); Serial.write(", ");
    Serial.write("xphaseInc "); Serial.print(target.xphaseInc); Serial.write(", ");
    Serial.write("yphaseInc "); Serial.print(target.yphaseInc); Serial.write(", ");
    Serial.write("xdest "); Serial.print(target.xdest); Serial.write(", ");
    Serial.write("ydest "); Serial.print(target.ydest); Serial.write(", ");
    Serial.write("delay "); Serial.print(target.delay); Serial.write(", ");
    Serial.write("delayCnt "); Serial.print(target.delayCnt); Serial.write(", ");
    Serial.write("xenable "); Serial.print(target.xenable); Serial.write(", ");
    Serial.write("yenable "); Serial.print(target.yenable); Serial.write(", ");
    Serial.write("\r\n");
    return;
  }
  if (curCmd[0] == 'S' && curCmd[1] == 'P') {
    int p = 1;
    sscanf(&curCmd[3], "%d", &p);
    if (p) {
      penUp();
    }
    else {
      penDown();
    }
    return;
  }
  if (curCmd[0] == 'S' && curCmd[1] == 'C') {
    //  I don't care about the config options
    return;
  }
  if (curCmd[0] == 'S' && curCmd[1] == 'M') {
    //  move
    int ms = 0, xt = 0, yt = 0;
    sscanf(&curCmd[3], "%d,%d,%d", &ms, &xt, &yt);
    target.setTarget(target.xpos + xt, target.ypos + yt);
    //  todo: change loopDelay
    return;
  }
  if (curCmd[0] == 'P' && (curCmd[1] == 'D' || curCmd[1] == 'O')) {
    //  engraver crap
    return;
  }
  if (curCmd[0] == 'S' && curCmd[1] == 'M') {
    //  wait milliseconds
    int ms = 1;
    sscanf(&curCmd[3], "%d", &ms);
    stepsToSkip += ms;
    return;
  }
  if (curCmd[0] == 'E' && curCmd[1] == 'M') {
    int moa = 1, mob = 1;
    sscanf(&curCmd[3], "%d,%d", &moa, &mob);
    target.enableMotors(moa, mob);
    return;
  }
  Serial.write("Unknown command: ");
  Serial.write(curCmd);
  Serial.write("\r\n");
}

void loop()
{
  while (!stepsToSkip && Serial.available()) {
    if (nextPtr > 31) {
      abortError("Too long command");
      return;
    }
    //  don't read more than a line
    if (nextPtr > 0 && nextCmd[nextPtr-1] == 13) {
      break;
    }
    nextCmd[nextPtr] = Serial.read();
    ++nextPtr;
  }
  if (target.step() && !stepsToSkip) {
    if (nextPtr > 0 && nextCmd[nextPtr-1] == 13) {
      //  ready for next command -- previous was run!
      memcpy(curCmd, nextCmd, sizeof(curCmd));
      curCmd[nextPtr-1] = 0;
      nextPtr = 0;
      doCommand();
      Serial.print("OK\r\n");
    }
  }
  
  //  test for pen mode button
  bool a0r = digitalRead(A0);
  if (a0r != a0state) {
    if (!a0r) {
      if (penIsUp) {
        penDown();
      }
      else {
        penUp();
      }
    }
    a0state = a0r;
  }
  
  if (stepsToSkip > 0) {
    --stepsToSkip;
  }
  delayMicroseconds(loopDelay);
}

