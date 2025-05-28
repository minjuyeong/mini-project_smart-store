#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// LCD 설정
LiquidCrystal_I2C lcd(0x27, 16, 2); // 주소 0x27, 16x2 LCD

// 키패드 설정
const byte ROWS = 4; 
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {5, 4, 3, 2};
byte colPins[COLS] = {6, 7, 8, 9};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// 비밀번호 설정
String password = "1234";
String input = "";

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("passward :      ");
}

void loop() {
  char key = keypad.getKey();
  
  if (key) {
    if (key == '#') {  // 입력 완료
      lcd.setCursor(0, 1);
      if (input == password) {
        lcd.print("OK!             ");
      } else {
        lcd.print("That's Wrong !  ");
      }
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("passward :      ");
      input = "";
    } else if (key == '*') {  // 입력 초기화
      input = "";
      lcd.setCursor(0, 1);
      lcd.print("                ");
    } else if (input.length() < 4) { // 입력 중
      input += key;
      lcd.setCursor(input.length() - 1, 1);
      lcd.print(key);
    }
  }
}



















