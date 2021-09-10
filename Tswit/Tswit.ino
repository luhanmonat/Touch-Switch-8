/*
 * Touch Switch access to CAP1298 
 * 9/21 Luhan Monat
 * Coded for Arduino Nano
 */

//  define port and bits used on nano
//  change these for other boards

#define PORT      PORTD
#define PORTIN    PIND
#define PORT_DIR  DDRD


//  I2C access lines

#define SCL   2       // I2C clock
#define SDA   3       // I2C data
#define INT   4       // hit detected (low)

#define IACK  0
#define NACK  1


void setup() {
  Serial.begin(9600);
  bitSet(PORT_DIR,SCL);
  bitSet(PORT_DIR,SDA);
  delayMicroseconds(20);
  I2CStop();
  I2CWbyte(0);
  TSWrite(0,0x00);        // reset normal sensitivity
  TSWrite(0x30,0x70);     // threshholds
  TSWrite(0x1F,0x4f);     // reduce sensitivty (2F)
  TSWrite(0x28,0x00);     // no auto repeats
  TSWrite(0x44,0x41);     // INT on press only
  TSWrite(0x60,0x07);     // Power button on #8
  TSWrite(0x61,0x26);     // Power button enable 1.1 second
}


// Test out the functionality of the software

void loop() {
byte  data;

 Printx("\nArmed\n",0);

next: 
  data=TSButton();
  if(!data)
    goto next;
  Printx("\nSensor: %02X",data);
  goto next; 

}

// Substitute for Arduino not having printf()
// Takes only 1 variable

void  Printx(char *fmt, int dat) {
char  ary[40];

  sprintf(ary,fmt,dat);
  Serial.print(ary);
}


// Read Touch Sensors 1-8
// Return 0 for none hit

byte  TSButton() {
byte  data,i;

  if(bitRead(PORTIN,INT))   // read the harware ALERT/INT line
    return(0);              // return if none detected
  TSWrite(0,0x00);          // Clear the results
  data=TSRead(3);           // (finger is still there) get all 8 sensor bits
  for(i=0;i<8;i++) {
    if(bitRead(data,i))     // scan for activ bit nr.
      return(i+1);          // range = 1-8
  }
  return(9);                // this should not ever happen
}


//  Read specified touch switch register using I2C protocol

byte  TSRead(byte reg) {
byte data;

  I2CStart();           // must always have START
  I2CWbyte(0x50);       // write mode
  I2CWbyte(reg);        // uses single byte for register address
  I2CStart();           // another START
  I2CWbyte(0x51);       // now go into read mode
  data=I2CRbyte(NACK);  // read just 1 byte w/no ACK
  I2CStop();            // always end with STOP
  return(data);
}

// Write specified data to specified register

byte  TSWrite(byte reg,byte dat) {

    I2CStart();         // always have START
    I2CWbyte(0x50);     // write mode
    I2CWbyte(reg);      // uses single byte for register address
    I2CWbyte(dat);      // write to register
    I2CStop();          // must always end with STOP
}


//  ******************************************
//  ******** Standard I2C functions **********
//  ******************************************

void I2CStart() {
  
  bitSet(PORT_DIR,SCL); x();    // clock is always output
  bitSet(PORT,SDA); x();        // data line to output mode
  bitSet(PORT,SCL); x();        // set clock high
  bitClear(PORT,SDA); x();      // lower data line for start condition
  bitClear(PORT,SCL); x();      // now set clock low
}

void I2CStop() {

  bitSet(PORT,SCL); x();         // raise clock line first
  bitSet(PORT,SDA); x();         // then raise data line for stop condition
}

// Write one byte via I2C

byte  I2CWbyte(byte dat) {
volatile byte  i,k,mask;

  mask=0x80;
  for(i=0;i<8;i++) {
    if(dat&mask)                // check current data bit
      bitSet(PORT,SDA);         // transfer to data line if high
    else
      bitClear(PORT,SDA);       // ... or low
    x();
    bitSet(PORT,SCL); x();      // raise clock line
    mask>>=1;                   // shift the mask bit
    bitClear(PORT,SCL); x();    // lower clock line
  }
 
  bitClear(PORT_DIR,SDA); x();  // Set for input
  bitSet(PORT,SCL); x();        // clock line high
  k=bitRead(PORTIN,SDA); x();   // Read ACK bit
  bitClear(PORT,SCL); x();      // lower the clock
  bitSet(PORT_DIR,SDA); x();    // default to data output
  bitClear(PORT,SDA); x();      // leave with data line low
  return(k);                    // return with ACK bit
}


// read byte with ACK unless nack.

byte  I2CRbyte(byte nack) {
byte  i,k,dat;

  bitClear(PORT_DIR,SDA); x();    // set data line for input
  dat=0;                          // z out initial data value
  for(i=0;i<8;i++) {              // set for 8 count
    x();
    bitSet(PORT,SCL); x();        // raise clock line
    k=bitRead(PORTIN,SDA); x();   // then read the data line
    bitClear(PORT,SCL); x();      // lower clock line
    dat<<=1;                      // data is MSB first so shift left
    bitWrite(dat,0,k);            // put the next bit into value
  }
  bitSet(PORT_DIR,SDA); x();      // switch to data output
  if(nack)                        // do we ACK this time?
    bitSet(PORT,SDA);             // high = no ACK
  else
    bitClear(PORT,SDA);           // low = ACK
  x();
  bitSet(PORT,SCL); x();          // start of clock pulse
  bitClear(PORT,SCL); x();        // end clock pulse
  bitClear(PORT,SDA); x();        // return with data line low
  return(dat);                    // return data
}

// just a small delay to keep clocks under 400khz

void  x() {
volatile byte z;

 delayMicroseconds(5);

}
