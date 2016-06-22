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
#define ON 0x6
#define PHARRAY 150
#define MPH1 1.7857
#define BPH1 -3.4821
#define MPH2 1.7857
#define BPH2 -3.4821
#define MPHC 1.7857
#define BPHC -3.4821
#define timeLong 1000  // seconds
#define OneSecond 1000 // milliseconds

byte ModeSet = 0;
byte Mode = 0;
byte ModeChange = 0;
byte Startup = 0;
byte Menu = 0;
byte R1Start = 0;
byte R2Start = 0;
byte RCStart = 0;
byte Set = 0;
double pH1min = 5.50;
double pH1start = 7.00;
double pH2min = 5.50;
double pH2start = 7.00;
double pHCmin = 5.50;
double pHCstart = 7.00;
double pH1read = 7.00;
double pH2read = 7.00;
double pHCread = 7.00;
unsigned long previousMillis = 0;
int pH1[PHARRAY];// Array to read in pH values
int pH2[PHARRAY];
int pHC[PHARRAY];
int pH1average = 0; // Average pH array value
int pH2average = 0;
int pHCaverage = 0;
// Initialize Counters
unsigned int Counter = 1;
unsigned int eCounter;
unsigned int pH1Counter = 1;
unsigned int pH2Counter = 1;
unsigned int pHCCounter = 1;
unsigned int DC1 = 0;
unsigned int DC2 = 0;
unsigned int DCC = 0;
// Initialize Setpoints
unsigned int BAM1 = 0;
unsigned int EFF1 = 0;
unsigned int BAM2 = 0;
unsigned int EFF2 = 0;
unsigned int BAMC = 0;
unsigned int EFFC = 0;
// Startup Setpoints
unsigned int BAM1start = 0;
unsigned int EFF1start = 0;
unsigned int BAM2start = 0;
unsigned int EFF2start = 0;
unsigned int BAMCstart = 0;
unsigned int EFFCstart = 0;
// Initialize Pump Setting Placeholders
unsigned int setBAM1 = 0;
unsigned int setEFF1 = 0;
unsigned int setBAM2 = 0;
unsigned int setEFF2 = 0;
unsigned int setBAMC = 0;
unsigned int setEFFC = 0;
// Pump Booleans
boolean Ba = LOW;
boolean Bb = LOW;
boolean Bc = LOW;
boolean Ea = LOW;
boolean Eb = LOW;
boolean Ec = LOW;
boolean pa = LOW;
boolean pb = LOW;
boolean pc = LOW;
double setpH1 = 0;
double setpH2 = 0;
double setpHC = 0;

void setup() {
  // Debugging output
  Serial.begin(9600);
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.setBacklight(ON);
  lcd.setCursor(0,0);
  lcd.print("Initialize Mode:");
  lcd.setCursor(0,1);
  lcd.print("Inoculate");
  
  // pH Sensors
  pinMode(A0, INPUT); // 1
  pinMode(A5, INPUT); // 2
  pinMode(A10, INPUT); // C
  // R1 Pumps
  pinMode(31, OUTPUT); // BAM1
  pinMode(32, OUTPUT); // EFF1
  pinMode(33, OUTPUT); // pHD1
  // R2 Pumps
  pinMode(36, OUTPUT); // BAM2
  pinMode(37, OUTPUT); // EFF2
  pinMode(38, OUTPUT); // pHD2
  // RC Pumps
  pinMode(41, OUTPUT); // BAMC
  pinMode(42, OUTPUT); // EFFC
  pinMode(43, OUTPUT); // pHDC
  // Initial Digital Setpoints
  digitalWrite(31, LOW);
  digitalWrite(32, LOW);
  digitalWrite(33, LOW);
  digitalWrite(36, LOW);
  digitalWrite(37, LOW);
  digitalWrite(38, LOW);
  digitalWrite(41, LOW);
  digitalWrite(42, LOW);
  digitalWrite(43, LOW);
}

void pHcontrol(double pH1compare, double pH2compare, double pHCcompare) {
  delay(50); // Rest
  byte i = 0;
  while (i < PHARRAY) {
    pH1[i] = analogRead(0);
    pH2[i] = analogRead(5);
    pHC[i] = analogRead(10);
    i++;
  }
  pH1average = mean(pH1, PHARRAY);
  pH1read = MPH1 * (pH1average / 100.00) + BPH1;
  pH2average = mean(pH2, PHARRAY);
  pH2read = MPH2 * (pH2average / 100.00) + BPH2;
  pHCaverage = mean(pHC, PHARRAY);
  pHCread = MPHC * (pHCaverage / 100.00) + BPHC;
  /* Collect 150 pH data points and average to check pH 
     measurement. If pHread is less than pHmin, dose 4M
     NaOH for 50ms */
  if (pH1read < pH1compare && pH1Counter >= 25) {
    pa = HIGH;
    DC1++;
    pH1Counter = 1;
    R1Start = 1;
  }
  if (pH2read < pH2compare && pH2Counter >= 25) {
    pb = HIGH;
    DC2++;
    pH2Counter = 1;
    R2Start = 10;
  }
  if (pHCread < pHCcompare && pHCCounter >= 25) {
    pc = HIGH;
    DCC++;
    pHCCounter = 1;
    RCStart = 100;
  }
  digitalWrite(33, pa);
  digitalWrite(38, pb);
  digitalWrite(43, pc);
  delay(50);
  digitalWrite(33, LOW);
  digitalWrite(38, LOW);
  digitalWrite(43, LOW);
  pa = LOW;
  pb = LOW;
  pc = LOW;
}

void pumpcontrol(unsigned int BAM1, unsigned int BAM2, unsigned int BAMC, unsigned int EFF1, unsigned int EFF2, unsigned int EFFC) {
  if (Counter <= 5) {
    eCounter = 1000;
  }
  else {
    eCounter = Counter - 5;
  }
  if (Counter <= BAM1) {
    Ba = HIGH;
  }
  if (Counter <= BAM2) {
    Bb = HIGH;
  }
  if (Counter <= BAMC) {
    Bc = HIGH;
  }
  if (eCounter <= EFF1) {
    Ea = HIGH;
  }
  if (eCounter <= EFF2) {
    Eb = HIGH;
  }
  if (eCounter <= EFFC) {
    Ec = HIGH;
  }
  digitalWrite(31, Ba);
  digitalWrite(32, Ea);
  digitalWrite(36, Bb);
  digitalWrite(37, Eb);
  digitalWrite(41, Bc);
  digitalWrite(42, Ec);
  delay (100);
  digitalWrite(31, LOW);
  digitalWrite(32, LOW);
  digitalWrite(36, LOW);
  digitalWrite(37, LOW);
  digitalWrite(41, LOW);
  digitalWrite(42, LOW);
  Ba = LOW;
  Bb = LOW;
  Bc = LOW;
  Ea = LOW;
  Eb = LOW;
  Ec = LOW;
}

uint8_t i=0;

void loop() {
  /* First select whether to run a startup sequence 
  or imediately begin in continuous */
  while (ModeSet == 0) {
    uint8_t buttons = lcd.readButtons();
    switch (Mode) {
      
      case 0: // Inoculate Mode
        if (buttons & BUTTON_LEFT) {
          Mode = 3; // Mode = 3
          lcd.clear();
          lcd.print("Initialize Mode:");
          lcd.setCursor(0,1);
          lcd.print("Batch");
        }
        else if (buttons & BUTTON_RIGHT) {
          Mode++; // Mode = 1
          lcd.clear();
          lcd.print("Initialize Mode:");
          lcd.setCursor(0,1);
          lcd.print("Start Up");
        }
        else if (buttons & BUTTON_SELECT) {
          ModeSet = 1;
        }
        break;
        
      case 1: // Start Up Mode
        if (buttons & BUTTON_LEFT) {
          Mode--; // Mode = 0
          lcd.clear();
          lcd.print("Initialize Mode:");
          lcd.setCursor(0,1);
          lcd.print("Inoculate");
        }
        else if (buttons & BUTTON_RIGHT) {
          Mode++; // Mode = 2
          lcd.clear();
          lcd.print("Initialize Mode:");
          lcd.setCursor(0,1);
          lcd.print("Continuous");
        }
        else if (buttons & BUTTON_SELECT) {
          ModeSet = 1;
        }
        break;    
        
      case 2: // Continuous Mode
        if (buttons & BUTTON_LEFT) {
          Mode--; // Mode = 1
          lcd.clear();
          lcd.print("Initialize Mode:");
          lcd.setCursor(0,1);
          lcd.print("Start Up");
        }
        else if (buttons & BUTTON_RIGHT) {
          Mode++; // Mode = 3
          lcd.clear();
          lcd.print("Initialize Mode:");
          lcd.setCursor(0,1);
          lcd.print("Batch");
        }
        else if (buttons & BUTTON_SELECT) {
          ModeSet = 1;
        }
        break;
        
      case 3: // Batch Mode
        if (buttons & BUTTON_LEFT) {
          Mode--; // Mode = 2
          lcd.clear();
          lcd.print("Initialize Mode:");
          lcd.setCursor(0,1);
          lcd.print("Continuous");
        }
        else if (buttons & BUTTON_RIGHT) {
          Mode = 0; // Mode = 0
          lcd.clear();
          lcd.print("Initialize Mode:");
          lcd.setCursor(0,1);
          lcd.print("Inoculate");
        }
        else if (buttons & BUTTON_SELECT) {
          ModeSet = 1;
        }
      break;
    }
  }
  
  // Always run the system and pH counters
  unsigned long currentMillis = millis();
  unsigned long t = currentMillis - previousMillis;
  if (t >= OneSecond) { 
    previousMillis = currentMillis;
    Counter++;
    pH1Counter++;
    pH2Counter++;
    pHCCounter++;
    /* Calculates seconds since last Counter increment.
       Once it has been greater than or equal to 1 second
       since the previous increment, the counter increases
       and previousMillis resets */
  if (Counter > timeLong) {
    Counter = 1;
  }
  // Resets Counter to 1 after each 1000 seconds
  
    switch (Mode) {
   
      case 0: // Inoculate
        pHcontrol(pH1start, pH2start, pHCstart);
      break;
      
      case 1: // Start Up
      
        Startup = R1Start + R2Start + RCStart;
      
        switch (Startup) {
       
          case 0: // All > 5.5
            pHcontrol(pH1min, pH2min, pHCmin);
          break;
        
          case 1: // R1 = 5.5, R2 & RC > 5.5
            pHcontrol(pH1min, pH2min, pHCmin);
            pumpcontrol(BAM1, BAM2start, BAMCstart, EFF1, EFF2start, EFFCstart);
          break;
        
          case 10: // R2 = 5.5, R1 & RC > 5.5
            pHcontrol(pH1min, pH2min, pHCmin);
            pumpcontrol(BAM1start, BAM2, BAMCstart, EFF1start, EFF2, EFFCstart);
          break;
        
          case 11: // R1 & R2 = 5.5, RC > 5.5
            pHcontrol(pH1min, pH2min, pHCmin);
            pumpcontrol(BAM1, BAM2, BAMCstart, EFF1, EFF2, EFFCstart);
          break;
        
          case 100: // RC = 5.5, R1 & R2 > 5.5
            pHcontrol(pH1min, pH2min, pHCmin);
            pumpcontrol(BAM1start, BAM2start, BAMC, EFF1start, EFF2start, EFFC);
          break;
        
          case 101: // R1 & RC = 5.5, R2 > 5.5
            pHcontrol(pH1min, pH2min, pHCmin);
            pumpcontrol(BAM1, BAM2start, BAMC, EFF1, EFF2start, EFFC);
          break;
        
          case 110: // R2 & RC = 5.5, R1 > 5.5
            pHcontrol(pH1min, pH2min, pHCmin);
            pumpcontrol(BAM1start, BAM2, BAMC, EFF1start, EFF2, EFFC);
          break;
        
          case 111: // All = 5.5
          Mode = 2;
          break;
        }    
      break;
    
      case 2: // Continuous
        pHcontrol(pH1min, pH2min, pHCmin);
        pumpcontrol(BAM1, BAM2, BAMC, EFF1, EFF2, EFFC);
      break;
    
      case 3: // Batch
        pHcontrol(pH1min, pH2min, pHCmin);
      break;
    }

  }
  
  uint8_t buttons = lcd.readButtons();
  
  // Menu options
  switch (Menu) {
   
    case 0: // Home Screen
      if (buttons & BUTTON_LEFT) {
        Menu = 13; // Dose Count C
        lcd.clear();
        lcd.print("Control DC");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // R1 BAM
        lcd.clear();
        lcd.print("R1 BAM");
      }
      else {
        lcd.setCursor(0,0);
        if (Counter < 10) {
          lcd.print("R1:"); lcd.print(pH1read); lcd.print(" R2:"); lcd.print(pH2read);
          lcd.setCursor(0,1);
          lcd.print("RC:"); lcd.print(pHCread); lcd.print(" LT:"); lcd.print("000"); lcd.print(Counter);
        }
        else if (Counter < 100) {
          lcd.print("R1:"); lcd.print(pH1read); lcd.print(" R2:"); lcd.print(pH2read);
          lcd.setCursor(0,1);
          lcd.print("RC:"); lcd.print(pHCread); lcd.print(" LT:"); lcd.print("00"); lcd.print(Counter);
        }
        else if (Counter < 1000) {
          lcd.print("R1:"); lcd.print(pH1read); lcd.print(" R2:"); lcd.print(pH2read);
          lcd.setCursor(0,1);
          lcd.print("RC:"); lcd.print(pHCread); lcd.print(" LT:"); lcd.print("0"); lcd.print(Counter);
        }
        else {
          lcd.print("R1:"); lcd.print(pH1read); lcd.print(" R2:"); lcd.print(pH2read);
          lcd.setCursor(0,1);
          lcd.print("RC:"); lcd.print(pHCread); lcd.print(" LT:"); lcd.print(Counter);
        }
      }
    break;
    
    case 1: // R1 BAM
      if (buttons & BUTTON_LEFT) {
        Menu--; // Home Screen
        lcd.clear();
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // R1 EFF
        lcd.clear();
        lcd.print("R1 EFF");
      }
      else if (buttons & BUTTON_SELECT) {
        setBAM1 = BAM1;
        Menu = 14; // Enter Settings Menu
        Set = 1; // BAM1 Settings
        lcd.clear();
        lcd.print("BAM1 Pump (s)");
        lcd.setCursor(0,1);
        lcd.print(setBAM1);
      }
    break;
    
    case 2: // R1 EFF
      if (buttons & BUTTON_LEFT) {
        Menu--; // R1 BAM
        lcd.clear();
        lcd.print("R1 BAM");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // R1 pH
        lcd.clear();
        lcd.print("R1 pH");
      }
      else if (buttons & BUTTON_SELECT) {
        setEFF1 = EFF1;
        Menu = 14; // Enter Settings Menu
        Set = 2; // EFF1 Settings
        lcd.clear();
        lcd.print("EFF1 Pump (s)");
        lcd.setCursor(0,1);
        lcd.print(setEFF1);
      }
    break;
    
    case 3: // R1 pH
      if (buttons & BUTTON_LEFT) {
        Menu--; // R1 EFF
        lcd.clear();
        lcd.print("R1 EFF");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // R2 BAM
        lcd.clear();
        lcd.print("R2 BAM");
      }
      else if (buttons & BUTTON_SELECT) {
        setpH1 = pH1min;
        Menu = 14; // Enter Settings Menu
        Set = 3; // pH1 Settings
        lcd.clear();
        lcd.print("pH1 min");
        lcd.setCursor(0,1);
        lcd.print(setpH1);
      }
    break;
    
    case 4: // R2 BAM
      if (buttons & BUTTON_LEFT) {
        Menu--; // R1 pH
        lcd.clear();
        lcd.print("R1 pH");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // R2 EFF
        lcd.clear();
        lcd.print("R2 EFF");
      }
      else if (buttons & BUTTON_SELECT) {
        setBAM2 = BAM2;
        Menu = 14; // Enter Settings Menu
        Set = 4; // BAM2 Settings
        lcd.clear();
        lcd.print("BAM2 Pump (s)");
        lcd.setCursor(0,1);
        lcd.print(setBAM2);
      }
    break;
    
    case 5: // R2 EFF
      if (buttons & BUTTON_LEFT) {
        Menu--; // R2 BAM
        lcd.clear();
        lcd.print("R2 BAM");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // R2 pH
        lcd.clear();
        lcd.print("R2 pH");
      }
      else if (buttons & BUTTON_SELECT) {
        setEFF2 = EFF2;
        Menu = 14; // Enter Settings Menu
        Set = 5; // EFF2 Settings
        lcd.clear();
        lcd.print("EFF2 Pump (s)");
        lcd.setCursor(0,1);
        lcd.print(setEFF2);
      }
    break;
    
    case 6: // R2 pH
      if (buttons & BUTTON_LEFT) {
        Menu--; // R2 EFF
        lcd.clear();
        lcd.print("R2 EFF");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // RC BAM
        lcd.clear();
        lcd.print("RC BAM");
      }
      else if (buttons & BUTTON_SELECT) {
        setpH2 = pH2min;
        Menu = 14; // Enter Settings Menu
        Set = 6; // pH2 Settings
        lcd.clear();
        lcd.print("pH2 min");
        lcd.setCursor(0,1);
        lcd.print(setpH2);
      }
    break;
    
    case 7: // RC BAM
      if (buttons & BUTTON_LEFT) {
        Menu--; // R2 pH
        lcd.clear();
        lcd.print("R2 pH");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // RC EFF
        lcd.clear();
        lcd.print("RC EFF");
      }
      else if (buttons & BUTTON_SELECT) {
        setBAMC = BAMC;
        Menu = 14; // Enter Settings Menu
        Set = 7; // BAMC Settings
        lcd.clear();
        lcd.print("BAMC Pump (s)");
        lcd.setCursor(0,1);
        lcd.print(setBAMC);
      }
    break;
    
    case 8: // RC EFF
      if (buttons & BUTTON_LEFT) {
        Menu--; // RC BAM
        lcd.clear();
        lcd.print("RC BAM");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // RC pH
        lcd.clear();
        lcd.print("RC pH");
      }
      else if (buttons & BUTTON_SELECT) {
        setEFFC = EFFC;
        Menu = 14; // Enter Settings Menu
        Set = 8; // EFFC Settings
        lcd.clear();
        lcd.print("EFFC Pump (s)");
        lcd.setCursor(0,1);
        lcd.print(setEFFC);
      }
    break;
    
    case 9: // RC pH
      if (buttons & BUTTON_LEFT) {
        Menu--; // RC EFF
        lcd.clear();
        lcd.print("RC EFF");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // Mode
        lcd.clear();
        lcd.print("Select Mode");
      }
      else if (buttons & BUTTON_SELECT) {
        setpHC = pHCmin;
        Menu = 14; // Enter Settings Menu
        Set = 9; // pHC Settings
        lcd.clear();
        lcd.print("pHC min");
        lcd.setCursor(0,1);
        lcd.print(setpHC);
      }
    break;
    
    case 10: // Mode
      if (buttons & BUTTON_LEFT) {
        Menu--; // RC pH
        lcd.clear();
        lcd.print("RC pH");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // Dose Count 1
        lcd.clear();
        lcd.print("Reactor 1 DC");
        lcd.setCursor(0,1);
        lcd.print(DC1);
      }
      else if (buttons & BUTTON_SELECT) {
        Menu = 14; // Enter Settings Menu
        Set = 10; // Mode Settings
        ModeChange = Mode;
        lcd.setCursor(0,1);
        if (ModeChange == 0) {
          lcd.print("Inoculate");
        }
        else if (ModeChange == 1) {
          lcd.print("Start Up");
        }
        else if (ModeChange == 2) {
          lcd.print("Continuous");
        }
        else if (ModeChange == 3) {
          lcd.print("Batch");
        }
      }
    break;
    
    case 11: // Dose Count 1
      lcd.setCursor(0,1);
      lcd.print(DC1);
      if (buttons & BUTTON_LEFT) {
        Menu--; // Mode
        lcd.clear();
        lcd.print("Select Mode");
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // Dose Count 2
        lcd.clear();
        lcd.print("Reactor 2 DC");
        lcd.setCursor(0,1);
        lcd.print(DC2);
      }
      else if (buttons & BUTTON_SELECT) {
        DC1 = 0;
        lcd.clear();
        lcd.print("Reactor 1 DC");
        lcd.setCursor(0,1);
        lcd.print(DC1);   
      } 
    break;
    
    case 12: // Dose Count 2
      lcd.setCursor(0,1);
      lcd.print(DC2);
      if (buttons & BUTTON_LEFT) {
        Menu--; // Dose Count 1
        lcd.clear();
        lcd.print("Reactor 1 DC");
        lcd.setCursor(0,1);
        lcd.print(DC1);
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu++; // Dose Count C
        lcd.clear();
        lcd.print("Control DC");
        lcd.setCursor(0,1);
        lcd.print(DCC);
      }
      else if (buttons & BUTTON_SELECT) {
        DC2 = 0;
        lcd.clear();
        lcd.print("Reactor 2 DC");
        lcd.setCursor(0,1);
        lcd.print(DC2);
      }
    break;
    
    case 13: // Dose Count C
      lcd.setCursor(0,1);
      lcd.print(DCC);
      if (buttons & BUTTON_LEFT) {
        Menu--; // Dose Count 2
        lcd.clear();
        lcd.print("Reactor 2 DC");
        lcd.setCursor(0,1);
        lcd.print(DC2);
      }
      else if (buttons & BUTTON_RIGHT) {
        Menu = 0; // Home Screen
        lcd.clear();
        lcd.setCursor(0,0);
        if (Counter < 10) {
          lcd.print("R1:"); lcd.print(pH1read); lcd.print(" R2:"); lcd.print(pH2read);
          lcd.setCursor(0,1);
          lcd.print("RC:"); lcd.print(pHCread); lcd.print(" LT:"); lcd.print("000"); lcd.print(Counter);
        }
        else if (Counter < 100) {
          lcd.print("R1:"); lcd.print(pH1read); lcd.print(" R2:"); lcd.print(pH2read);
          lcd.setCursor(0,1);
          lcd.print("RC:"); lcd.print(pHCread); lcd.print(" LT:"); lcd.print("00"); lcd.print(Counter);
        }
        else if (Counter < 1000) {
          lcd.print("R1:"); lcd.print(pH1read); lcd.print(" R2:"); lcd.print(pH2read);
          lcd.setCursor(0,1);
          lcd.print("RC:"); lcd.print(pHCread); lcd.print(" LT:"); lcd.print("0"); lcd.print(Counter);
        }
        else {
          lcd.print("R1:"); lcd.print(pH1read); lcd.print(" R2:"); lcd.print(pH2read);
          lcd.setCursor(0,1);
          lcd.print("RC:"); lcd.print(pHCread); lcd.print(" LT:"); lcd.print(Counter);
        }
      }
      else if (buttons & BUTTON_SELECT) {
        DCC = 0;
        lcd.clear();
        lcd.print("Control DC");
        lcd.setCursor(0,1);
        lcd.print(DCC);
      }
    break;
    
    case 14: //Settings Screen
      switch (Set) {
        
        case 0: // Do nothing
        break;
        
        case 1: // R1 BAM
          if (buttons & BUTTON_UP) {
            setBAM1++;
            if (setBAM1 > 1000) {
              setBAM1 = 0;
              lcd.clear();
              lcd.print("BAM1 Pump (s)");
            }
          }
          else if (buttons & BUTTON_DOWN) {
            if (setBAM1 == 0) {
              setBAM1 = 1000;
            }
            else {
              setBAM1--;
              if (setBAM1 == 999 || setBAM1 == 99 || setBAM1 == 9) {
                lcd.clear();
                lcd.print("BAM1 Pump (s)");
              }
            }           
          }
          else if (buttons & BUTTON_SELECT) {
            BAM1 = setBAM1;
            Menu = 0;
            Set = 0;
          }
          if (Menu == 14) {
            lcd.setCursor(0, 1);
            lcd.print(setBAM1);
          }
          else if (Menu == 0) {
            lcd.clear();
          }
        break;
    
        case 2: // R1 EFF
          if (buttons & BUTTON_UP) {
            setEFF1++;
            if (setEFF1 > 1000) {
              setEFF1 = 0;
              lcd.clear();
              lcd.print("EFF1 Pump (s)");
            }
          }
          else if (buttons & BUTTON_DOWN) {
            if (setEFF1 == 0) {
              setEFF1 = 1000;
            }
            else {
              setEFF1--;
              if (setEFF1 == 999 || setEFF1 == 99 || setEFF1 == 9) {
                lcd.clear();
                lcd.print("EFF1 Pump (s)");
              }
            }           
          }
          else if (buttons & BUTTON_SELECT) {
            EFF1 = setEFF1;
            Menu = 0;
            Set = 0;
          }
          if (Menu == 14) {
            lcd.setCursor(0, 1);
            lcd.print(setEFF1);
          }
          else if (Menu == 0) {
            lcd.clear();
          }
        break;
    
        case 3: // R1 pH
          if (buttons & BUTTON_UP) {
            setpH1 = setpH1 + 0.01;
            if (setpH1 > 14) {
              setpH1 = 0;
              lcd.clear();
              lcd.print("pH1 min");
            }
          }
          else if (buttons & BUTTON_DOWN) {
            setpH1 = setpH1 - 0.01;
            if (setpH1 == 9.99) {
              lcd.clear();
              lcd.print("pH1 min");
            }
            else if (setpH1 < 0) {
              setpH1 = 14.00;
            }
          }
          else if (buttons & BUTTON_SELECT) {
            pH1min = setpH1;
            Menu = 0;
            Set = 0;
          }
          if (Menu == 14) {
            lcd.setCursor(0, 1);
            lcd.print(setpH1);
          }
          else if (Menu == 0) {
            lcd.clear();
          }
        break;
    
        case 4: // R2 BAM
          if (buttons & BUTTON_UP) {
            setBAM2++;
            if (setBAM2 > 1000) {
              setBAM2 = 0;
              lcd.clear();
              lcd.print("BAM2 Pump (s)");
            }
          }
          else if (buttons & BUTTON_DOWN) {
            if (setBAM2 == 0) {
              setBAM2 = 1000;
            }
            else {
              setBAM2--;
              if (setBAM2 == 999 || setBAM2 == 99 || setBAM2 == 9) {
                lcd.clear();
                lcd.print("BAM2 Pump (s)");
              }
            }           
          }
          else if (buttons & BUTTON_SELECT) {
            BAM2 = setBAM2;
            Menu = 0;
            Set = 0;
          }
          if (Menu == 14) {
            lcd.setCursor(0, 1);
            lcd.print(setBAM2);
          }
          else if (Menu == 0) {
            lcd.clear();
          }
        break;
    
        case 5: // R2 EFF
          if (buttons & BUTTON_UP) {
            setEFF2++;
            if (setEFF2 > 1000) {
              setEFF2 = 0;
              lcd.clear();
              lcd.print("EFF2 Pump (s)");
            }
          }
          else if (buttons & BUTTON_DOWN) {
            if (setEFF2 == 0) {
              setEFF2 = 1000;
            }
            else {
              setEFF2--;
              if (setEFF2 == 999 || setEFF2 == 99 || setEFF2 == 9) {
                lcd.clear();
                lcd.print("EFF2 Pump (s)");
              }
            }         
          }
          else if (buttons & BUTTON_SELECT) {
            EFF2 = setEFF2;
            Menu = 0;
            Set = 0;
          }
          if (Menu == 14) {
            lcd.setCursor(0, 1);
            lcd.print(setEFF2);
          }
          else if (Menu == 0) {
            lcd.clear();
          }
        break;
    
        case 6: // R2 pH
          if (buttons & BUTTON_UP) {
            setpH2 = setpH2 + 0.01;
            if (setpH2 > 14) {
              setpH2 = 0;
              lcd.clear();
              lcd.print("pH2 min");
            }
          }
          else if (buttons & BUTTON_DOWN) {
            setpH2 = setpH2 - 0.01;
            if (setpH2 == 9.99) {
              lcd.clear();
              lcd.print("pH2 min");
            }
            else if (setpH2 < 0) {
              setpH2 = 14.00;
            }
          }
          else if (buttons & BUTTON_SELECT) {
            pH2min = setpH2;
            Menu = 0;
            Set = 0;
          }
          if (Menu == 14) {
            lcd.setCursor(0, 1);
            lcd.print(setpH2);
          }
          else if (Menu == 0) {
            lcd.clear();
          }
        break;
    
        case 7: // RC BAM
          if (buttons & BUTTON_UP) {
            setBAMC++;
            if (setBAMC > 1000) {
              setBAMC = 0;
              lcd.clear();
              lcd.print("BAMC Pump (s)");
            }
          }
          else if (buttons & BUTTON_DOWN) {
            if (setBAMC == 0) {
              setBAMC = 1000;
            }
            else {
              setBAMC--;
              if (setBAMC == 999 || setBAMC == 99 || setBAMC == 9) {
                lcd.clear();
                lcd.print("BAMC Pump (s)");
              }
            }
          }
          else if (buttons & BUTTON_SELECT) {
            BAMC = setBAMC;
            Menu = 0;
            Set = 0;
          }
          if (Menu == 14) {
            lcd.setCursor(0, 1);
            lcd.print(setBAMC);
          }
          else if (Menu == 0) {
            lcd.clear();
          }
        break;
    
        case 8: // RC EFF
          if (buttons & BUTTON_UP) {
            setEFFC++;
            if (setEFFC > 1000) {
              setEFFC = 0;
              lcd.clear();
              lcd.print("EFFC Pump (s)");
            }
          }
          else if (buttons & BUTTON_DOWN) {
            if (setEFFC == 0) {
              setEFFC = 1000;
            }
            else {
              setEFFC--;
              if (setEFFC == 999 || setEFFC == 99 || setEFFC == 9) {
                lcd.clear();
                lcd.print("EFFC Pump (s)");
              }
            }
          }
          else if (buttons & BUTTON_SELECT) {
            EFFC = setEFFC;
            Menu = 0;
            Set = 0;
          }
          if (Menu == 14) {
            lcd.setCursor(0, 1);
            lcd.print(setEFFC);
          }
          else if (Menu == 0) {
            lcd.clear();
          }
        break;
    
        case 9: // RC pH
          if (buttons & BUTTON_UP) {
            setpHC = setpHC + 0.01;
            if (setpHC > 14) {
              setpHC = 0;
              lcd.clear();
              lcd.print("pHC min");
            }
          }
          else if (buttons & BUTTON_DOWN) {
            setpHC = setpHC - 0.01;
            if (setpHC == 9.99) {
              lcd.clear();
              lcd.print("pHC min");
            }
            else if (setpHC < 0) {
              setpHC = 14.00;
            }
          }
          else if (buttons & BUTTON_SELECT) {
            pHCmin = setpHC;
            Menu = 0;
            Set = 0;
          }
          if (Menu == 14) {
            lcd.setCursor(0, 1);
            lcd.print(setpHC);
          }
          else if (Menu == 0) {
            lcd.clear();
          }
        break;
    
        case 10: // Mode
          switch (ModeChange) {
            case 0: // Inoculate
              if (buttons & BUTTON_UP) {
                ModeChange++;
                lcd.clear();
                lcd.print("Select Mode");
                lcd.setCursor(0,1);
                lcd.print("Start Up");
              }
              else if (buttons & BUTTON_DOWN) {
                ModeChange = 3;
                lcd.clear();
                lcd.print("Select Mode");
                lcd.setCursor(0,1);
                lcd.print("Batch");
              }
              else if (buttons & BUTTON_SELECT) {
                Mode = ModeChange;
                Menu = 0;
                Set = 0;
              }
            break;
            
            case 1: // Start Up
              if (buttons & BUTTON_UP) {
                ModeChange++;
                lcd.clear();
                lcd.print("Select Mode");
                lcd.setCursor(0,1);
                lcd.print("Continuous");
              }
              else if (buttons & BUTTON_DOWN) {
                ModeChange--;
                lcd.clear();
                lcd.print("Select Mode");
                lcd.setCursor(0,1);
                lcd.print("Inoculate");
              }
              else if (buttons & BUTTON_SELECT) {
                Mode = ModeChange;
                Menu = 0;
                Set = 0;
              }
            break;
            
            case 2: // Continuous
              if (buttons & BUTTON_UP) {
                ModeChange++;
                lcd.clear();
                lcd.print("Select Mode");
                lcd.setCursor(0,1);
                lcd.print("Batch");
              }
              else if (buttons & BUTTON_DOWN) {
                ModeChange--;
                lcd.clear();
                lcd.print("Select Mode");
                lcd.setCursor(0,1);
                lcd.print("Start Up");
              }
              else if (buttons & BUTTON_SELECT) {
                Mode = ModeChange;
                Menu = 0;
                Set = 0;
              }
            break;
            
            case 3: // Batch
              if (buttons & BUTTON_UP) {
                ModeChange = 0;
                lcd.clear();
                lcd.print("Select Mode");
                lcd.setCursor(0,1);
                lcd.print("Inoculate");
              }
              else if (buttons & BUTTON_DOWN) {
                ModeChange--;
                lcd.clear();
                lcd.print("Select Mode");
                lcd.setCursor(0,1);
                lcd.print("Continuous");
              }
              else if (buttons & BUTTON_SELECT) {
                Mode = ModeChange;
                Menu = 0;
                Set = 0;
              }
            break;
            
            default:
              Menu = 0;
              Set = 0;
            break;
          }
        break;
        
        default:
          Menu = 0;
          Set = 0;
        break;
      }
    break;
    
    default:
      Menu = 0;
      Set = 0;
    break;
  }
}
