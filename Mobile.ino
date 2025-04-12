#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

// Initialize SoftwareSerial for SIM900A communication
SoftwareSerial Serial1(2, 3); // RX, TX (D2, D3)

// Initialize LiquidCrystal for LCD
LiquidCrystal lcd(14, 15, 16, 17, 18, 19); // RS, E, D4, D5, D6, D7

// Custom character for back button
byte back[8] = {
  0b00000,
  0b00000,
  0b11111,
  0b10101,
  0b11011,
  0b11111,
  0b00000,
  0b00000
};

// Variables for SMS and call handling
String number = "";
String msg = "";
String instr = "";
String str_sms = "";
String str1 = "";
int ring = 0;
int i = 0, temp = 0;
int sms_flag = 0;
char sms_num[3];
int rec_read = 0;
int temp1 = 0;

// Keypad configuration
const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {11, 10, 9, 8}; // connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4}; // connect to the column pinouts of the keypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// Character map for SMS input
String ch = "1,.?!@abc2def3ghi4jkl5mno6pqrs7tuv8wxyz90 ";

void setup() {
  Serial1.begin(9600);
  lcd.begin(16, 2);
  lcd.createChar(1, back);
  lcd.print("Simple Mobile ");
  lcd.setCursor(0, 1);
  lcd.print("System Ready..");
  delay(1000);
  gsm_init();
  lcd.clear();
  lcd.print("System Ready");
  delay(2000);
}

void loop() {
  serialEvent();
  if (sms_flag == 1) {
    lcd.clear();
    lcd.print("New Message");
    int ind = instr.indexOf("+CMTI: \"SM\",");
    ind += 12;
    int k = 0;
    lcd.setCursor(0, 1);
    lcd.print(ind);
    while (1) {
      while (instr[ind] != 0x0D) {
        sms_num[k++] = instr[ind++];
      }
      break;
    }
    ind = 0;
    sms_flag = 0;
    lcd.setCursor(0, 1);
    lcd.print("Read SMS --> D");
    delay(4000);
    instr = "";
    rec_read = 1;
    temp1 = 1;
    i = 0;
  }
  if (ring == 1) {
    number = "";
    int loc = instr.indexOf("+CLIP: \"");
    if (loc > 0) {
      number += instr.substring(loc + 8, loc + 13 + 8);
    }
    lcd.setCursor(0, 0);
    lcd.print("Incoming...    ");
    lcd.setCursor(0, 1);
    lcd.print(number);
    instr = "";
    i = 0;
  } else {
    serialEvent();
    lcd.setCursor(0, 0);
    lcd.print("Call --> C      ");
    lcd.setCursor(0, 1);
    lcd.print("SMS  --> B   ");
    if (rec_read == 1) {
      lcd.write(1);
      lcd.print("   ");
    } else
      lcd.print("     ");
  }
  char key = customKeypad.getKey();
  if (key) {
    if (key == 'A') {
      if (ring == 1) {
        Serial1.println("ATA");
        delay(5000);
      }
    } else if (key == 'C') {
      call();
    } else if (key == 'B') {
      sms();
    } else if (key == 'D' && temp1 == 1) {
      rec_read = 0;
      lcd.clear();
      lcd.print("Please wait...");
      Serial1.print("AT+CMGR=");
      Serial1.println(sms_num);
      int sms_read_flag = 1;
      str_sms = "";
      while (sms_read_flag) {
        while (Serial1.available() > 0) {
          char ch = Serial1.read();
          str_sms += ch;
          if (str_sms.indexOf("OK") > 0) {
            sms_read_flag = 0;
          }
        }
      }
      int l1 = str_sms.indexOf("\"\\r\\n");
      int l2 = str_sms.indexOf("OK");
      String sms = str_sms.substring(l1 + 3, l2 - 4);
      lcd.clear();
      lcd.print(sms);
      delay(5000);
    }
    delay(1000);
  }
}

void call() {
  number = "";
  lcd.clear();
  lcd.print("After Enter No.");
  lcd.setCursor(0, 1);
  lcd.print("Press C to Call");
  delay(2000);
  lcd.clear();
  lcd.print("Enter Number:");
  lcd.setCursor(0, 1);
  while (1) {
    serialEvent();
    char key = customKeypad.getKey();
    if (key) {
      if (key == 'C') {
        lcd.clear();
        lcd.print("Calling........");
        lcd.setCursor(0, 1);
        lcd.print(number);
        Serial1.print("ATD");
        Serial1.print(number);
        Serial1.println(";");
        long stime = millis() + 5000;
        int ans = 1;
        while (ans == 1) {
          while (Serial1.available() > 0) {
            if (Serial1.find("OK")) {
              lcd.clear();
              lcd.print("Ringing....");
              int l = 0;
              str1 = "";
              while (ans == 1) {
                while (Serial1.available() > 0) {
                  char ch = Serial1.read();
                  str1 += ch;
                  if (str1.indexOf("NO CARRIER") > 0) {
                    lcd.clear();
                    lcd.print("Call End");
                    delay(2000);
                    ans = 0;
                    return;
                  }
                }
                char key = customKeypad.getKey();
                if (key == 'D') {
                  lcd.clear();
                  lcd.print("Call End");
                  delay(2000);
                  ans = 0;
                  return;
                }
                if (ans == 0)
                  break;
              }
            }
          }
        }
      } else {
        number += key;
        lcd.print(key);
      }
    }
  }
}

void sms() {
  lcd.clear();
  lcd.print("Initializing SMS");
  Serial1.println("AT+CMGF=1");
  delay(400);
  lcd.clear();
  lcd.print("After Enter No.");
  lcd.setCursor(0, 1);
  lcd.print("Press D        ");
  delay(2000);
  lcd.clear();
  lcd.print("Enter Rcpt No.:");
  lcd.setCursor(0, 1);
  Serial1.print("AT+CMGS=\"");
  while (1) {
    serialEvent();
    char key = customKeypad.getKey();
    if (key) {
      if (key == 'D') {
        Serial1.println("\"");
        break;
      } else {
        Serial1.print(key);
        lcd.print(key);
      }
    }
  }
  lcd.clear();
  lcd.print("After Enter MSG ");
  lcd.setCursor(0, 1);
  lcd.print("Press D to Send ");
  delay(2000);
  lcd.clear();
  lcd.print("Enter Your Msg");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  alfakey();
}

void alfakey() {
  int x = 0, y = 0;
  int num = 0;
  while (1) {
    lcd.cursor();
    char key = customKeypad.getKey();
    if (key) {
      if (key == '1') {
        num = 0;
        lcd.setCursor(x, y);
        lcd.print(ch[num]);
        for (int i = 0; i < 3000; i++) {
          lcd.noCursor();
          char key = customKeypad.getKey();
          if (key == '1') {
            num++;
            if (num > 5)
              num = 0;
            lcd.setCursor(x, y);
            lcd.print(ch[num]);
            i = 0;
            delay(200);
          }
        }
        x++;
        if (x > 15) {
          x = 0;
          y++;
          y %= 2;
        }
        msg += ch[num];
      }
    }
  }
}

void serialEvent() {
  while (Serial1.available()) {
    char c = Serial1.read();
    instr += c;
    if (c == '\n') {
      // Process the received command
      // Reset the instruction string for the next command
      instr = "";
    }
  }
}

void gsm_init() {
  // Initialize GSM module
  Serial1.println("AT");
  delay(100);
  Serial1.println("AT+CSQ");
  delay(100);
  Serial1.println("AT+CREG?");
  delay(100);
}