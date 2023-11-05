
#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK  (13)
#define BMP_MISO (12)
#define BMP_MOSI (11)
#define BMP_CS   (10)


Adafruit_BMP280 bmp; // I2C
//Adafruit_BMP280 bmp(BMP_CS); // hardware SPI
//Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

// LCD1602 or LCD2004 settings



#define HMC5883_WriteAddress 0x1E //  i.e 0x3C >> 1
#define HMC5883_ModeRegisterAddress 0x02
#define HMC5883_ContinuousModeCommand (uint8_t)0x00     // cast to uint8_t added to get code to compile under Arduino v1.0
#define HMC5883_DataOutputXMSBAddress 0x03
#define I2C_TX write
#define I2C_RX read
#include <LiquidCrystal_I2C.h>
// Set the LCD address to 0x3F for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
// end of LCD1602 or LCD2004 settings

unsigned long loopCount;
unsigned long startTime;
String msg;
int regb = 0x01;
int regbdata = 0x40;
int outputData[6];

int x = 1;
int df;
int f = 0; //initializing floor
int i = 0;
int val;
int count = 0;
int current_floor;

char dd_332[] = {'W', 'S', 'W', 'N', 'W', 'N'};
int dd2_332[] = {10, 15, 12, 17, 20, 21};

char dd_423[] = {'W', 'S', 'E', 'S', 'E'};
int dd2_423[] = {10, 15, 12, 17, 22, 21, 7};

char dd_511[] = {'W', 'S', 'W', 'S'};
int dd2_511[] = {10, 15, 12, 17};

const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};


Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);


void reached() {
  x = 4;
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Reached");
  delay(1000);

}

void altcheck() {

  lcd.clear();// clean previous values from screen
  lcd.setCursor (0, 0); //character 9, line 2
  static float fixed = bmp.readAltitude(1013.25); //declaring static variable for initial value of altitude
  float temp = bmp.readAltitude(1013.25); //storing current value
  float dif = temp - fixed; //altitude difference
  lcd.setCursor(0, 0);
  lcd.print("D.F:");
  lcd.setCursor(4, 0);
  lcd.print(df);
  lcd.setCursor(0, 1);
  lcd.print("C floor:");
  lcd.setCursor(9, 1);
  lcd.print(f);
  lcd.setCursor(9, 0);
  lcd.print("R:");
  lcd.setCursor(11, 0);
  lcd.print(val);
  if (dif > 2 && dif < 4 && count == 0) {
    f++;
    count = 1;
  }
  else if (dif > 5 && dif < 7 && count == 1) {
    f++;
    count = 2;
  }
  else if (dif > 8 && dif < 10 && count == 2) {
    f++;
    count = 3;
  }
  else if (dif > 11 && dif < 14 && count == 3) {
    f++;
    count = 4;
  }
  else if (dif > 15 && dif < 17 && count == 4) {
    f++;
    count = 5;
  }
  else if (dif > 18 && dif < 20 && count == 5) {
    f++;
  }
  else{}
  delay(1000);
}



void compass1() {
  int x, y, z;
  double angle;
  lcd.clear();
  delay(100); // Wait for the screen to clear
  Serial.write(254);
  Serial.write(128); //Goto line-1
  Wire.beginTransmission(HMC5883_WriteAddress);
  Wire.I2C_TX(regb);
  Wire.I2C_TX(regbdata);
  Wire.endTransmission();

  delay(100);
  Wire.beginTransmission(HMC5883_WriteAddress); //Initiate a transmission with HMC5883 (Write address).
  Wire.I2C_TX(HMC5883_ModeRegisterAddress);     //Place the Mode Register Address in send-buffer.
  Wire.I2C_TX(HMC5883_ContinuousModeCommand);   //Place the command for Continuous operation Mode in send-buffer.
  Wire.endTransmission();                       //Send the send-buffer to HMC5883 and end the I2C transmission.
  delay(100);

  Wire.beginTransmission(HMC5883_WriteAddress);  //Initiate a transmission with HMC5883 (Write address).
  Wire.requestFrom(HMC5883_WriteAddress, 6);     //Request 6 bytes of data from the address specified.

  delay(100);

  //Read the value of magnetic components X,Y and Z

  if (Wire.available() <= 6) // If the number of bytes available for reading is <=6
  {
    for (int i = 0; i < 6; i++)
    {
      outputData[i] = Wire.I2C_RX(); //Store the data in outputData buffer
    }
  }

  x = outputData[0] << 8 | outputData[1]; //Combine MSB and LSB of X Data output register
  z = outputData[2] << 8 | outputData[3]; //Combine MSB and LSB of Z Data output register
  y = outputData[4] << 8 | outputData[5]; //Combine MSB and LSB of Y Data output register
  angle = (double)atan2(y, x); // angle in radians


  float declinationAngle = -0.019;
  angle += declinationAngle;

  // Correct for when signs are reversed.
  if (angle < 0)    angle += 2 * PI;

  // Check for wrap due to addition of declination.
  if (angle > 2 * PI) angle -= 2 * PI;

  // Convert radians to degrees for readability.
  float bearing = angle * 180 / PI;

  //Serial.println();
  //Serial.println("Heading (degrees): " + String(bearing));

  //Print the angle on the LCD Screen
  Serial.write(254);
  Serial.write(128); //Goto line 1
  //lcd.print("A:"+ String(angle,2));

  dir1(); //calling fucntion

  Serial.write(254);
  Serial.write(192); //Goto line 2
  //Print the approximate direction on LCD
  lcd.setCursor(10, 0);
  lcd.print("C.D: ");
  lcd.setCursor(14, 0);

  if ((bearing < 22.5)  || (bearing > 337.5 ))  lcd.print("W");
  if ((bearing > 22.5)  && (bearing < 67.5 ))   lcd.print("NW");
  if ((bearing > 67.5)  && (bearing < 112.5 ))  lcd.print("N");
  if ((bearing > 112.5) && (bearing < 157.5 ))  lcd.print("NE");
  if ((bearing > 157.5) && (bearing < 202.5 ))  lcd.print("E");
  if ((bearing > 202.5) && (bearing < 247.5 ))  lcd.print("SE");
  if ((bearing > 247.5) && (bearing < 292.5 ))  lcd.print("S");
  if ((bearing > 292.5) && (bearing < 337.5 ))  lcd.print("SW");
  delay(1000);

  //Print the approximate direction
  //Serial.print("\nYou are heading ");
  // if ((bearing > 337.5) || (bearing < 22.5))    Serial.print("west");
  // if ((bearing > 22.5)  && (bearing < 67.5 ))   Serial.print("North-west");
  // if ((bearing > 67.5)  && (bearing < 112.5 ))  Serial.print("North");
  //  if ((bearing > 112.5) && (bearing < 157.5 ))  Serial.print("North-East");
  //  if ((bearing > 157.5) && (bearing < 202.5 ))  Serial.print("East");
  //  if ((bearing > 202.5) && (bearing < 247.5 ))  Serial.print("South-East");
  //  if ((bearing > 247.5) && (bearing < 292.5 ))  Serial.print("south");
  //  if ((bearing > 292.5) && (bearing < 337.5 ))  Serial.print("South-West");

}

void dir1() {
  char customKey = customKeypad.getKey();

  if (val == 332) {
    lcd.setCursor(10, 1);
    lcd.print("R:");
    lcd.setCursor(12, 1);
    lcd.print("332");
    lcd.setCursor(0, 0);
    lcd.print("Dest:");
    lcd.setCursor(5, 0);
    lcd.print(dd_332[i]);
    lcd.setCursor(0, 1);
    lcd.print("Step:");
    lcd.setCursor(5, 1);
    lcd.print(dd2_332[i]);
    if (customKey) {
      if (customKey == '#' || customKey == '*' ) {
        i++;
      }
      if (i > 5) {
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("Reached");
        delay(1000);
        x = 1;
        start1();
      }
    }
  } else if (val == 423) {
    lcd.setCursor(10, 1);
    lcd.print("R:");
    lcd.setCursor(12, 1);
    lcd.print("423");
    lcd.setCursor(0, 0);
    lcd.print("Dest:");
    lcd.setCursor(5, 0);
    lcd.print(dd_423[i]);
    lcd.setCursor(0, 1);
    lcd.print("Step:");
    lcd.setCursor(5, 1);
    lcd.print(dd2_423[i]);
    if (customKey) {
      if (customKey == '#' || customKey == '*') {
        i++;
      }
      if (i > 4) {
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("Reached");
        delay(1000);
        x = 1;
        start1();
      }
    }
  } else if (val == 511) {
    lcd.setCursor(10, 1);
    lcd.print("R:");
    lcd.setCursor(12, 1);
    lcd.print("511");
    lcd.setCursor(0, 0);
    lcd.print("Dest:");
    lcd.setCursor(5, 0);
    lcd.print(dd_511[i]);
    lcd.setCursor(0, 1);
    lcd.print("Step:");
    lcd.setCursor(5, 1);
    lcd.print(dd2_511[i]);
    if (customKey) {
      if (customKey == '#' || customKey == '*') {
        i++;
      }
      if (i > 3) {
        lcd.clear();
        lcd.setCursor(4, 0);
        lcd.print("Reached");
        //lcd.clear();
        delay(1000);
        x = 1;
        start1();
      }
    }
  }
}

int floor1(int val) {

  if (val == 332) {
    df = 3;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("D.F:");
    lcd.setCursor(4, 0);
    lcd.print(df);
    // Serial.println(val);
    lcd.setCursor(11, 0);   lcd.print("R:");
    lcd.setCursor(13, 0);   lcd.print(val);
    x = 3;
  }
  else if (val == 423)
  {
    df = 4;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("D.F:");
    lcd.setCursor(4, 0);
    lcd.print(df);
    lcd.setCursor(11, 0);   lcd.print("R:");
    lcd.setCursor(13, 0);   lcd.print(val);
    x = 3;
  }
  else if (val == 511) {
    df = 5;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("D.F:");
    lcd.setCursor(4, 0);
    lcd.print(df);
    lcd.setCursor(11, 0);   lcd.print("R:");
    lcd.setCursor(13, 0);   lcd.print(val);
    x = 3;
  }
  else {
    lcd.clear();
    lcd.print("Wrong input");
    delay(500);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Room number:");
    x = 2;
  }
  delay(1000);

}

void start1() {

  df = 0;
  f = 0; //initializing floor
  i = 0;
  val = 0;
  current_floor = 0;
  char customKey = customKeypad.getKey();
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("UIU ROOM");
  lcd.setCursor(5, 1);
  lcd.print("Finder");
  delay(2500);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Insert C.F:");
  f = getKeypadIntegerMulti();
  x = 2;
}

long getKeypadIntegerMulti()
{

  long value = 0;                                // the number accumulator
  long keyvalue;                                     // the key pressed at current moment
  int isnum;
  //Serial.println("Enter the digits,press any non-digit to end ");
  //Serial.print("You have typed: ");



  do
  {
    keyvalue = customKeypad.getKey();                          // input the key
    isnum = (keyvalue >= '0' && keyvalue <= '9');         // is it a digit?
    if (keyvalue == 'C') {
      lcd.clear();
      loop();

    }

    if (isnum)
    {
      // Serial.print(keyvalue - '0');
      lcd.print(keyvalue - '0');
      value = value * 10 + keyvalue - '0';               // accumulate the input number
    }

  } while (isnum || !keyvalue);                          // until not a digit or while no key pressed
  //
  //Serial.println(" ");
  //lcd.println(" ");
  //Serial.print("Returning from funtion: ");
  // Serial.println(value);
  //  lcd.println(value);
  return value;

}//getKeypadInteg

void setup() {
  Serial.begin(9600);

  // initialize the LCD,
  lcd.begin();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.clear();
  lcd.setCursor (0, 0); //

  if (!bmp.begin()) {
    lcd.print("BMP280 Error");
    while (1);
  }
  //lcd.print("BMP280 Test");
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void loop() {

  if (x == 1) {
    start1();
  }
  else if (x == 2) {
    char customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'C') {
        lcd.clear();
        x = 1;
      }
    }
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Room number:");
    val = getKeypadIntegerMulti();
    floor1(val);
  }
  else if (x == 3) {
    char customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'C') {
        lcd.clear();
        x = 1;
      }
    }

    altcheck();
    if (f == df) {
      reached();
    }
  } else if (x == 4) {
    char customKey = customKeypad.getKey();
    if (customKey) {
      if (customKey == 'C') {
        lcd.clear();
        x = 1;
      }
    }

    compass1();
  }
}// loop end

/*
   Written by Ahmad Shamshiri
   Jan 22, 2019 in Ajax, Ontario, Canada
   getTemp(char type)
   returns temperature in either C, F or K
   @param type is character of upper case
   C is used to get Celsius
   F is used to get fahrenheit
   K is used for Kelvin
   Written by Ahmad Shamshiri for Robojax.com
*/
float getTemp(char type) {
  float c = bmp.readTemperature();//get main temperature in C
  float f = c * 9.0 / 5.0 + 32;// convert to fahrenheit
  if (type == 'F')
  {
    return f;// fahrenheit
  } else if (type == 'K')
  {
    return c + 274.15;// return Kelvin
  } else {
    return c; //return Celsius
  }

}//getTemp ends
