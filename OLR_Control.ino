/*********************

Example code for the Adafruit RGB Character LCD Shield and Library

This code displays text on the shield, and also reads the buttons on the keypad.
When a button is pressed, the backlight changes color.

**********************/

// include the library code:
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>
#include <Average.h>

// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// These #defines make it easy to set the backlight color
#define ON 0x1
#define OFF 0x0
#define PHARRAY 150
#define EFFMAX 600
#define MPH 1.7857
#define BPH -3.4821

void setup() {
  // Debugging output
  Serial.begin(9600);
  pinMode(A15, INPUT); // pH sensor
  pinMode(40, INPUT);  // Level sensor
  pinMode(41, OUTPUT); // Effluent Pump
  pinMode(42, OUTPUT); // BAM Pump
  pinMode(43, OUTPUT); // SUB Pump
  pinMode(44, OUTPUT); // H2O Pump
  pinMode(45, OUTPUT); // pH Dosing Pump
  digitalWrite(41, LOW);
  digitalWrite(42, LOW);
  digitalWrite(43, LOW);
  digitalWrite(44, LOW);
  digitalWrite(45, LOW);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.print("MCF Loading Rate"); // Display program name
  lcd.setBacklight(ON);
}

uint8_t i=0;
byte Home = 0;
byte Set = 0;
double pHmin = 5.50;
double pHread = 7.00;
unsigned long previousMillis = 0;
int pH[PHARRAY]; // Array to read in pH values
int pHaverage = 0; // Average pH array value
unsigned int Counter = 1; // Initialize Counter at 1
unsigned int pHCounter = 1;
unsigned int EffOffCount = 1;
unsigned int EffOnCount = 1;
unsigned int BAM = 268;
unsigned int SUB = 31;
unsigned int H2O = 665; // Initialize pump counters
unsigned int setBAM = 0;
unsigned int setSUB = 0;
unsigned int setH2O = 0;
unsigned int DoseCount = 0;
double setpH = 0; // Variables for setting values within menus

#define timeLong 1000  // seconds
#define OneSecond 1000 // milliseconds

void loop() {
  unsigned long currentMillis = millis();
  unsigned long t = currentMillis - previousMillis;
  if (t >= OneSecond) { 
    previousMillis = currentMillis;
    Counter++;
    pHCounter++;
    /* Calculates seconds since last Counter increment.
       Once it has been greater than or equal to 1 second
       since the previous increment, the counter increases
       and previousMillis resets */
  if (Counter > timeLong) {
    Counter = 1;
  }
  // Resets Counter to 1 after each 1000 seconds
  
    if (Counter <= BAM && Counter <= SUB && Counter <= H2O) {
      digitalWrite(42, HIGH);
      digitalWrite(43, HIGH);
      digitalWrite(44, HIGH);
      delay(100);
      digitalWrite(42, LOW);
      digitalWrite(43, LOW);
      digitalWrite(44, LOW);
    }
    else if (Counter <= BAM && Counter > SUB && Counter <= H2O) {
      digitalWrite(42, HIGH);
      digitalWrite(44, HIGH);
      delay(100);
      digitalWrite(42, LOW);
      digitalWrite(44, LOW);
    }
    else if (Counter > BAM && Counter > SUB && Counter <= H2O) {
      digitalWrite(44, HIGH);
      delay(100);
      digitalWrite(44, LOW);
    }
    else if (Counter > BAM && Counter <= SUB && Counter > H2O) {
      digitalWrite(43, HIGH);
      delay(100);
      digitalWrite(43, LOW);
    }
    else if (Counter <= BAM && Counter <= SUB && Counter > H2O) {
      digitalWrite(42, HIGH);
      digitalWrite(43, HIGH);
      delay(100);
      digitalWrite(42, LOW);
      digitalWrite(43, LOW);
    }
    else if (Counter > BAM && Counter <= SUB && Counter <= H2O) {
      digitalWrite(43, HIGH);
      digitalWrite(44, HIGH);
      delay(100);
      digitalWrite(43, LOW);
      digitalWrite(44, LOW);
    }
    else if (Counter <= BAM && Counter > SUB && Counter > H2O) {
      digitalWrite(42, HIGH);
      delay(100);
      digitalWrite(42, LOW);
    }
  
    delay(50); // Rest
    byte i = 0;
    while (i < PHARRAY) {
      pH[i] = analogRead(15);
      i++;
    }
    pHaverage = mean(pH, PHARRAY);
    pHread = MPH * (pHaverage / 100.00) + BPH;
    if (pHread < pHmin && pHCounter >= 25) {
      digitalWrite(45, HIGH);
      delay(50);
      digitalWrite(45, LOW);
      DoseCount++;
      pHCounter = 1;
    }
    /* Collect 100 pH data points and average to check pH 
       measurement. If pHread is less than pHmin, dose 4M
       NaOH for 50ms */
    
    if (digitalRead(41) == LOW) { // Eff Pump Off
      EffOffCount++;
      if (Home != 7) {
        if (digitalRead(40) == LOW && EffOffCount > 2) { // Level High
          digitalWrite(41, HIGH); // Turn Eff Pump On
          EffOnCount = 0;
        }
      }
    }
    if (digitalRead(41) == HIGH) { // Eff Pump On
      EffOnCount++;
      if (digitalRead(40) == HIGH && EffOnCount > 2) { // Level Low
        digitalWrite(41, LOW); // Turn Eff Pump Off
        EffOffCount = 1;
      }
    }
    if (EffOnCount > EFFMAX) {
      Home = 7;
      BAM = 0;
      SUB = 0;
      H2O = 0;
      EffOnCount = 0;
      digitalWrite(41, LOW);
      lcd.clear();
      lcd.print("Eff Pump Error");
    }
     
  }

  uint8_t buttons = lcd.readButtons();
  
  switch (Home) {
    case 0: // Displaying MCF Loading Rate
      if (buttons & BUTTON_UP) {
        lcd.setBacklight(ON);
      }
      else if (buttons & BUTTON_DOWN) {
        lcd.setBacklight(OFF);
      }
      else if (buttons & BUTTON_RIGHT) {
        Home++; // Home = 1
        lcd.clear();
        lcd.print("BAM Pump (s)");
        lcd.setBacklight(ON);
      }
      else if (buttons & BUTTON_LEFT) {
        Home = 5; // Home = 5
        lcd.clear();
        lcd.print("DC ");lcd.print(DoseCount);
        lcd.setBacklight(ON);
      }
      lcd.setCursor(0, 1);
      if (Counter < 10) {
        lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("000"); lcd.print(Counter);
      }
      else if (Counter < 100) {
        lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("00"); lcd.print(Counter);
      }
      else if (Counter < 1000) {
        lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("0"); lcd.print(Counter);
      }
      else {
        lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print(Counter);
      }
      break;
      
      case 1: // Displaying BAM Pump (s)
      if (buttons & BUTTON_RIGHT) {
        Home++; // Home = 2
        lcd.clear();
        lcd.print("SUB Pump (s)");
      }
      else if (buttons & BUTTON_LEFT) {
        Home--; // Home = 0
        lcd.clear();
        lcd.print("MCF Loading Rate");
      }
      else if (buttons & BUTTON_SELECT) {
        Set = 1; // Enter BAM menu
        Home = 6; // Enter sub menus
        setBAM = BAM;
        lcd.clear();
        lcd.print("BAM Pump (s)");
        lcd.setCursor(0, 1);
        lcd.print(setBAM);
      }
      if (Home != 6) {
        lcd.setCursor(0, 1);
        if (Counter < 10) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("000"); lcd.print(Counter);
        }
        else if (Counter < 100) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("00"); lcd.print(Counter);
        }
        else if (Counter < 1000) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("0"); lcd.print(Counter);
        }
        else {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print(Counter);
        }
      }
      break;
      
      case 2: // Displaying SUB Pump (s)
      if (buttons & BUTTON_RIGHT) {
        Home++; // Home = 3
        lcd.clear();
        lcd.print("H2O Pump (s)");
      }
      else if (buttons & BUTTON_LEFT) {
        Home--; // Home = 1
        lcd.clear();
        lcd.print("BAM Pump (s)");
      }
      else if (buttons & BUTTON_SELECT) {
        Set = 2; // Enter SUB menu
        Home = 6; // Enter sub menus
        setSUB = SUB;
        lcd.clear();
        lcd.print("SUB Pump (s)");
        lcd.setCursor(0, 1);
        lcd.print(setSUB);
      }
      if (Home != 6) {
        lcd.setCursor(0, 1);
        if (Counter < 10) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("000"); lcd.print(Counter);
        }
        else if (Counter < 100) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("00"); lcd.print(Counter);
        }
        else if (Counter < 1000) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("0"); lcd.print(Counter);
        }
        else {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print(Counter);
        }
      }
      break;
      
      case 3: // Displaying H2O Pump (s)
      if (buttons & BUTTON_RIGHT) {
        Home++; // Home = 4
        lcd.clear();
        lcd.print("pH min");
      }
      else if (buttons & BUTTON_LEFT) {
        Home--; // Home = 2
        lcd.clear();
        lcd.print("SUB Pump (s)");
      }
      else if (buttons & BUTTON_SELECT) {
        Set = 3; // Enter H2O menu
        Home = 6; // Enter sub menus
        setH2O = H2O;
        lcd.clear();
        lcd.print("H2O Pump (s)");
        lcd.setCursor(0, 1);
        lcd.print(setH2O);
      }
      if (Home != 6) {
        lcd.setCursor(0, 1);
        if (Counter < 10) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("000"); lcd.print(Counter);
        }
        else if (Counter < 100) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("00"); lcd.print(Counter);
        }
        else if (Counter < 1000) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("0"); lcd.print(Counter);
        }
        else {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print(Counter);
        }
      }
      break;
      
      case 4: // Displaying pH min
      if (buttons & BUTTON_RIGHT) {
        Home++; // Home = 5
        lcd.clear();
        lcd.print("DC ");lcd.print(DoseCount);
      }
      else if (buttons & BUTTON_LEFT) {
        Home--; // Home = 3
        lcd.clear();
        lcd.print("H2O Pump (s)");
      }
      else if (buttons & BUTTON_SELECT) {
        Set = 4; // Enter pH menu
        Home = 6; // Enter sub menus
        setpH = pHmin;
        lcd.clear();
        lcd.print("pH min");
        lcd.setCursor(0, 1);
        lcd.print(setpH);
      }
      if (Home != 6) {
        lcd.setCursor(0, 1);
        if (Counter < 10) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("000"); lcd.print(Counter);
        }
        else if (Counter < 100) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("00"); lcd.print(Counter);
        }
        else if (Counter < 1000) {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("0"); lcd.print(Counter);
        }
        else {
          lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print(Counter);
        }
      }
      break;
      
      case 5: // Displaying DC xxxx
      lcd.setCursor(0, 0);
      lcd.print("DC ");lcd.print(DoseCount);
      if (buttons & BUTTON_RIGHT) {
        Home = 0; // Home = 0
        lcd.clear();
        lcd.print("MCF Loading Rate");
      }
      else if (buttons & BUTTON_LEFT) {
        Home--; // Home = 4
        lcd.clear();
        lcd.print("pH min");
      }
      else if (buttons & BUTTON_SELECT) {
        DoseCount = 0;
        lcd.clear();
        lcd.print("DC ");lcd.print(DoseCount);
      }
      lcd.setCursor(0, 1);
      if (Counter < 10) {
        lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("000"); lcd.print(Counter);
      }
      else if (Counter < 100) {
        lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("00"); lcd.print(Counter);
      }
      else if (Counter < 1000) {
        lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print("0"); lcd.print(Counter);
      }
      else {
        lcd.print("pH "); lcd.print(pHread); lcd.print(" LT "); lcd.print(Counter);
      }
      break;
      
      case 6: // Sub menus for pH and feed pumps
      switch (Set) {
        case 1: // BAM menu
        if (buttons & BUTTON_UP) {
          setBAM++;
          if (setBAM > 1000) {
            setBAM = 0;
            lcd.clear();
            lcd.print("BAM Pump (s)");
          }
        }
        else if (buttons & BUTTON_DOWN) {
          setBAM--;
          if (setBAM == 999 || setBAM == 99 || setBAM == 9) {
            lcd.clear();
            lcd.print("BAM Pump (s)");
          }
          else if (setBAM < 0) {
            setBAM = 1000;
          }
        }
        else if (buttons & BUTTON_SELECT) {
          BAM = setBAM;
          Home = 0;
          Set = 0;
        }
        if (Home == 6) {
          lcd.setCursor(0, 1);
          lcd.print(setBAM);
        }
        else if (Home == 0) {
          lcd.clear();
          lcd.print("MCF Loading Rate");
        }
        break;
        
        case 2: // SUB menu
        if (buttons & BUTTON_UP) {
          setSUB++;
          if (setSUB > 1000) {
            setSUB = 0;
            lcd.clear();
            lcd.print("SUB Pump (s)");
          }
        }
        else if (buttons & BUTTON_DOWN) {
          setSUB--;
          if (setSUB == 999 || setSUB == 99 || setSUB == 9) {
            lcd.clear();
            lcd.print("SUB Pump (s)");
          }
          else if (setSUB < 0) {
            setSUB = 1000;
          }
        }
        else if (buttons & BUTTON_SELECT) {
          SUB = setSUB;
          Home = 0;
          Set = 0;
        }
        if (Home == 6) {
          lcd.setCursor(0, 1);
          lcd.print(setSUB);
        }
        else if (Home == 0) {
          lcd.clear();
          lcd.print("MCF Loading Rate");
        }
        break;
        
        case 3: // H2O menu
        if (buttons & BUTTON_UP) {
          setH2O++;
          if (setH2O > 1000) {
            setH2O = 0;
            lcd.clear();
            lcd.print("H2O Pump (s)");
          }
        }
        else if (buttons & BUTTON_DOWN) {
          setH2O--;
          if (setH2O == 999 || setH2O == 99 || setH2O == 9) {
            lcd.clear();
            lcd.print("H2O Pump (s)");
          }
          else if (setH2O < 0) {
            setSUB = 1000;
          }
        }
        else if (buttons & BUTTON_SELECT) {
          H2O = setH2O;
          Home = 0;
          Set = 0;
        }
        if (Home == 6) {
          lcd.setCursor(0, 1);
          lcd.print(setH2O);
        }
        else if (Home == 0) {
          lcd.clear();
          lcd.print("MCF Loading Rate");
        }
        break;
        
        case 4: // pH menu
        if (buttons & BUTTON_UP) {
          setpH = setpH + 0.01;
          if (setH2O > 14) {
            setH2O = 0;
            lcd.clear();
            lcd.print("pH min");
          }
        }
        else if (buttons & BUTTON_DOWN) {
          setpH = setpH - 0.01;
          if (setpH == 9.99) {
            lcd.clear();
            lcd.print("pH min");
          }
          else if (setpH < 0) {
            setpH = 14.00;
          }
        }
        else if (buttons & BUTTON_SELECT) {
          pHmin = setpH;
          Home = 0;
          Set = 0;
        }
        if (Home == 6) {
          lcd.setCursor(0, 1);
          lcd.print(setpH);
        }
        else if (Home == 0) {
          lcd.clear();
          lcd.print("MCF Loading Rate");
        }
        break;
        
        case 7:
        if (buttons & BUTTON_SELECT) {
          Home = 0;
          Set = 0;
          EffOnCount = 0;
          BAM = setBAM;
          SUB = setSUB;
          H2O = setH2O;
          lcd.clear();
          lcd.print("MCF Loading Rate");
        }
        break;
        
        default:
        Home = 0;
        Set = 0;
        break;
      }
      break;
      
      default:
      Home = 0;
      Set = 0;
      break;
  }
}
