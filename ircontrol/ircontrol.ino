
//  Theory of operation:
//  Timer 2 generates a carrier frequency for remote control on pin 3.
//  This could be from 16 kHz to 60 kHz or so (not doing 437 kHz right now.)
//  MOSI (pin 11) gates the output of pin 3, and clock frequency is set to 125 kHz, 
//  giving 8 microsecond precision to the on/off of the gate (plus some latency
//  for re-filling the MOSI output byte)

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>

#define PIN_CARRIER 3
#define PIN_MODULATOR 11
#define PIN_STATUS 7
#define PIN_SPI_SS 10

#define CMD_RESET 0
//  no payload

#define CMD_STATUS 1
//  ushort version

#define CMD_CARRIER 2
//  ushort frequency
//  byte dutycycle
#define MIN_CARRIER 62 // Hz, see set_carrier()

#define CMD_CODES 3
//  byte numcodes

#define RESP_ERROR 1
//  data string

#define RESP_STATUS 2
//  data string

#define RESP_OK 3
//  command byte

//  max number of samples
unsigned short samples[200];
unsigned char nSamples = 0;

//  max command size
unsigned char cmdBuf[32];
unsigned char nCmd = 0;
unsigned long receiveMillis = 0;


void setup_spi();
void disable_spi();
void set_carrier(unsigned short carrier, unsigned char duty);

void setup() {
  wdt_reset();
  wdt_enable(WDTO_2S);

  pinMode(PIN_MODULATOR, OUTPUT);  //  PB3, OC2A, MOSI  --  modulator
  digitalWrite(PIN_MODULATOR, LOW);
  pinMode(13, OUTPUT);  //  PB5, SCK, Arduino status LED (unfortunate)
  digitalWrite(13, HIGH);
  pinMode(PIN_STATUS, OUTPUT);   //  PB5, SCK, application status LED
  //  flash indicator while booting
  digitalWrite(PIN_STATUS, HIGH);
  pinMode(PIN_SPI_SS, OUTPUT);  //  Configure as output to make sure SPI runs
  digitalWrite(PIN_SPI_SS, HIGH);

  pinMode(PIN_CARRIER, OUTPUT);   //  PD3, OC2B  --  carrier
  digitalWrite(PIN_CARRIER, LOW);

  //  initialize serial port
  Serial.begin(57600);

  //  set up carrier timer
  set_carrier(38000, 33);

  //  keep the LED on long enough to actually be seen
  delay(50);

  wdt_reset();
  delay(100);
  digitalWrite(13, LOW);
  digitalWrite(PIN_STATUS, LOW);
  delay(100);
  digitalWrite(13, HIGH);
  digitalWrite(PIN_STATUS, HIGH);
  delay(100);
  digitalWrite(13, LOW);  //  and then it turns into a clock
  digitalWrite(PIN_STATUS, LOW);
  
  //  set up SPI clock
  setup_spi();
}

void setup_spi() {
  //  turn off fast mode if it's on
  SPSR = 0;
  //  divide down to 125 kHz
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPHA) | (1 << SPR1) | (1 << SPR0);
}

unsigned char shiftBits[] = {
  0,
  3, // 8
  2, // 32
  1, // 64
  1, // 128
  1, // 256
  2, // 1024
};

void set_carrier(unsigned short carrier, unsigned char duty) {
  TIMSK2 = 0;  //  no interrupts from this timer
  ASSR = 0;    // use internal clock
  //  setup timer/counter 2 for PWM at "carrier" frequency with "duty" duty cycle
  TCCR2A = (1 << COM2B1) | (0 << COM2B0) // fast PWM, non-inverting
        | (1 << WGM21) | (1 << WGM20);     // fast PWM, OCRA cycle length
  unsigned short clk = 16000000 / carrier;
  unsigned char mode = 1;
  while ((clk > 256) && (mode < 7)) {
    clk = clk >> shiftBits[mode];
    mode += 1;
  }
  //  let's do the math:
  //  16000000/1024 == 15625
  //  15625/256 == 61.03
  //  So, 62 Hz is the minimum integer carrier
  if (clk > 256) {
    error("too low carrier");
  }
  TCCR2B = (1 << WGM22)                  // fast PWM, OCRA cycle length
        | (mode << CS20);  //  clock, no pre-scaling
  OCR2A = (unsigned char)(clk-1);
  OCR2B = (unsigned char)(clk * duty / 100);
}

void reset() {
  //  Set watchdog to something short-ish.
  //  Then wait around for watchdog to reset chip!
  wdt_enable(WDTO_1S);
  digitalWrite(11, LOW);  //  turn off carrier
  digitalWrite(3, LOW);   //  turn off modulator
  while (true) {
    //  wait for watchdog to reset chip, flashing light
    digitalWrite(8, HIGH);
    delay(75);
    digitalWrite(8, LOW);
    delay(100);
  }
}

void error(char const *msg) {
  digitalWrite(PIN_STATUS, HIGH);
  //  length, status, msg, crlf
  unsigned char len = strlen(msg) + 2;
  Serial.write(len);
  Serial.write(RESP_ERROR);
  Serial.print(msg);
  digitalWrite(PIN_STATUS, LOW);
  delay(50);  //  wait for output to drain
  reset();
}

void ok(unsigned char cmd) {
  Serial.write(3);
  Serial.write(RESP_OK);
  Serial.write(cmd);
}

//  could be done with a shift and a subtract
unsigned char bitvals[8] = {
  0x80,
  0x40,
  0x20,
  0x10,
  0x08,
  0x04,
  0x02,
  0x01
};

void blast_samples() {
  //  make MOSI modulate based on samples
  unsigned char mval = 0;
  unsigned short ticks = 0;
  //  write SPI just to get started
  SPDR = 0;
  for (unsigned char i = 0; i != nSamples || ticks != 0;) {
    //  generate new byte -- this must run faster than 64 microseconds!
    //  (does it? my guess is this takes about 30 us... but I should measure!)
    unsigned char nuByte = 0;
    for (unsigned char j = 0; j != 8; ++j) {
      if (ticks == 0) {
        if (i != nSamples) {
          ticks = samples[i];
          i += 1;
          if (ticks & 0x8000U) {
            mval = 0xff;
            ticks = ticks & 0x7fffU;
          }
          else {
            mval = 0;
          }
        }
        else {
          ticks = 8 - j;
        }
      }
      nuByte = nuByte | (bitvals[j] & mval);
      ticks -= 1;
    }
    //  disable interrupts -- I know I won't call this with interrupts disabled
    cli();
    //  wait for MOSI to complete
    while (!(SPSR & (1 << SPIF))) {
      // do nothing, with interrupts off!
    }
    //  write SPI
    SPDR = nuByte;
    //  enable interrupts -- I know I won't call this with interrupts disabled
    sei();
  }
  //  wait for pulses to clear
  while (!(SPSR & (1 << SPIF))) {
    // do nothing, with interrupts off!
  }
  digitalWrite(11, LOW);  //  turn off modulator for sure
}

void cmd_status() {
  //unsigned char flags = cmdBuf[2];
  //I recognize no flags
  char const *toWrite = "jwIR;" __DATE__ ";0";
  unsigned char len = 2 + strlen(toWrite);
  Serial.write(len);
  Serial.write(RESP_STATUS);
  Serial.print(toWrite);
}

void checkPayload(unsigned char len) {
  if (cmdBuf[0] != len + 2) {
    error("bad payload");
  }
}

void cmd_carrier() {
  checkPayload(3);
  unsigned short freq = (cmdBuf[2] << 8) + cmdBuf[3];
  if (freq < MIN_CARRIER) {
    error("bad freq");
  }
  unsigned char duty = cmdBuf[4];
  if (duty < 1 || duty > 99) {
    error("bad dutycycle");
  }
}

void cmd_codes() {
  checkPayload(1);
  unsigned char nCodes = cmdBuf[2];
  if (nCodes > sizeof(samples)/sizeof(samples[0])) {
    error("bad count");
  }
  unsigned long time = millis();
  for (unsigned char i = 0; i != nCodes; ++i) {
    if (Serial.available() < 2) {
      if (millis() - time > 1000) {
        //  pre-empt the watchdog -- this really should take much less than 100 ms
        error("receive timeout");
      }
      else {
        // do nothing
      }
    }
    else {
      unsigned char hi = (unsigned char)Serial.read();
      unsigned char lo = (unsigned char)Serial.read();
      samples[i] = (hi << 8) + lo;
    }
  }
  nSamples = nCodes;
  blast_samples();
}


void loop() {
  wdt_reset();
  while (Serial.available() && nCmd < sizeof(cmdBuf)) {
    receiveMillis = millis();
    cmdBuf[nCmd] = Serial.read();
    ++nCmd;
    if (cmdBuf[0] == 0) {
      //  illegal packet -- reboot chip!
      error("zero length");
      break;
    }
    if (cmdBuf[0] > sizeof(cmdBuf)) {
      //  illegal packet -- reboot chip!
      error("high length");
      break;
    }
    if (nCmd == cmdBuf[0]) {
      digitalWrite(PIN_STATUS, HIGH);
      //  got a full command
      switch (cmdBuf[1]) {
        case CMD_RESET:
          ok(CMD_RESET);
          reset();  //  doesn't return
          break;
        case CMD_STATUS:
          cmd_status();
          break;
        case CMD_CARRIER:
          cmd_carrier();
          break;
        case CMD_CODES:
          cmd_codes();
          break;
        default:
          //  illegal packet -- reboot chip!
          error("unknown cmd");  //  doesn't return
          break;
      }
      ok(cmdBuf[1]);
      nCmd = 0;
      digitalWrite(PIN_STATUS, LOW);
    }
  }
  if (nCmd > 0) {
     if (millis() - receiveMillis > 1000) {
       error("receive timeout");
     }
  }
  else {
    //  do nothing
    delay(5);
  }
}

