q#include <SoftwareSerial.h>
#include <Wire.h>
#include <MsTimer2.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
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

int stopFlag = 0;   //ALLOFF 명령어 보낼지 여부, send함수에서 사용

void setup()
{
  lcd_init(); //lcd 초기화 밑, 초기화면 설정
  BTSerial.begin(9600); //블루투스 설정
  MsTimer2::set(1000, timerIsr); // 타이머 설정
  MsTimer2::start();
}

void loop()
{
  if(getkey() != NULL)
  {
    keypad();
  }

  //타이머 인터럽트 1초마다 store_name 확인 후 동작하기
  if(BTSerial.available() && strcmp(store_name, '0') != 0)
  {
    send();
    recv();
  }
  else
  {
    lcd_clock();
  }
}

void lcd_init()
{
  lcd.init();
  lcd.backlight();
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);
}

void keypad()
{
  char key = keypad.getKey();
  
  if (key) {
    if (key == '#') {  // 입력 완료
      lcd.setCursor(0, 1);
      if (input == password) {
        lcd.print("OK!             ");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("select store");

        char storename = keypad.getkey(); //store A, B, C, D, allstop을 위한 구조
        if (storename != 'A' || storename != 'B' || storename != 'C' || storename != 'D')
        {
          lcd.clean();
          lcd.print("reselect store");
        } 
        else
        {
          stopFlag = 1;
        }
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

void send())
{
  /*
  if (stopFlag)
    store_name에 들어있는 가게로 보냄
    [store_name]StoreState 보냄
  else
    ALLSTOP
  */
}

void recv()
{
  /*
  [storename]온도@습도@led@lock@humanCount@FANPWM
  -> strtok() 이후로 lcd_update()
  */
}

void lcd_clock()
{
  lcd.print("clock display");
}

void lcd_uddate()
{
    /*
  [storename]온도@습도@led@lock@humanCount@FANPWM
  -> strtok() 이후로 lcd_update()
  */
}