#include <SoftwareSerial.h>
#include <Wire.h>
#include <MsTimer2.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define MSG_SIZE 60 //송수신하는 패킷 사이즈
#define pArray_CNT 15 //명령어의 사이즈
#define DEBUG
char recvId[10] = "MJY_LIN";  // SQL 저장 클라이이언트 ID
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

char storename = '\0';
char inputvalue;

float temp; //온도
float humi; //습도
int lockState;  //잠금상태  1이면 잠금 0이면 열림
int ledState; //led 밝기
int humanCount; //매장 내 고객 수
int fanPwm; //실링팬 회전 상태

int stopFlag = 0;   //ALLOFF 명령어 보낼지 여부, send함수에서 사용

SoftwareSerial BTSerial(10, 11);

volatile bool doUpdate = false;

void setup()
{
  #ifdef DEBUG
  Serial.begin(115200);
  Serial.println("setup() start!");
#endif
  lcd_init(); //lcd 초기화 밑, 초기화면 설정
  BTSerial.begin(9600); //블루투스 설정
  MsTimer2::set(1000, timerIsr); // 타이머 설정
  MsTimer2::start();
}

void loop()
{
  inputvalue = keypad.getKey();
  if(storename == '\0' && inputvalue != NULL)
  {
      keypadInput();
  }
  //타이머 인터럽트 1초마다 store_name 확인 후 동작하기
  else if (doUpdate && storename != '\0') {
    doUpdate = false;  // clear flag
    send();
    recv();
    lcd_update();
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
  lcd.clear();
}

void keypadInput()
{
  char abc[30] = {0};
  char prevKey = NO_KEY;
  storename = 0;
  input = "";
  lcd.setCursor(0, 0);
  lcd.print("passward :      ");
  while (1) {
    char key = keypad.getKey();
    if (key != NO_KEY && key != prevKey) {
    prevKey = key;
    if (key == '#') {  // 입력 완료
      lcd.setCursor(0, 1);
      if (input == password) {
        lcd.print("OK!             ");
        delay(2000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("select store");

        // char storename; //store A, B, C, D, allstop을 위한 구조
        do {
        storename = keypad.getKey();
        } while (storename == NO_KEY);
        if (storename == 'A' || storename == 'B' || storename == 'C' || storename == 'D') {
          stopFlag = 1;
          sprintf(abc,"%c store", storename);
          lcd.setCursor(0, 1);
          lcd.print(abc);
          return; // 성공 시 탈출
        } else {
          lcd.clear();
          lcd.print("reselect store");
          delay(2000);
          lcd.clear();
          return;
        }
      } else {
        lcd.print("That's Wrong !  ");
        delay(2000);
        lcd.clear();
        return;
      }
    } else if (key == '*') {  // 입력 초기화
      input = "";
      lcd.setCursor(0, 1);
      lcd.print("                ");
    } else if (input.length() < 4) { // 입력 중
      Serial.print("1");
      input += key;
      lcd.setCursor(input.length() - 1, 1);
      lcd.print(key);
    }
    }
    if (key == NO_KEY) {
    prevKey = NO_KEY; // 키가 떼어졌을 때만 초기화
    }
  }
}

void send()
{
  /*
  if (stopFlag)
    store_name에 들어있는 가게로 보냄
    [store_name]StoreState 보냄
  else
    ALLSTOP
  */
  char sendBuffer[MSG_SIZE]={0}; 

  if (stopFlag)
  {
    sprintf(sendBuffer, "[%s]STATE", storename);
  }
  else
    sprintf(sendBuffer, "[%s]ALLSTOP", storename);

  BTSerial.write(sendBuffer, strlen(sendBuffer));
}

void recv()
{
  /*
  [storename]온도@습도@led@lock@humanCount@FANPWM
  -> strtok() 이후로 lcd_update()
  */
  
  int i = 0;
  char * pToken;
  char * pArray[pArray_CNT] = {0};
  char recvBuffer[MSG_SIZE] = {0};
  int len = BTSerial.readBytesUntil('\n', recvBuffer, sizeof(recvBuffer) - 1);
  
  pToken = strtok(recvBuffer, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= pArray_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }
  if(!strcmp(pArray[0], "STORENAME"))
  {
    temp = atoi(pArray[1]);
    humi = atoi(pArray[2]);
    lockState = pArray[3];
    ledState = pArray[4];
    humanCount = pArray[5]; //매장 내 고객 수
    fanPwm = pArray[6]; //실링팬 회전 상태
  }
}

void lcd_clock()
{
  lcd.setCursor(0, 0);
  lcd.print("clock clock     ");
}

void lcd_update()
{
  char lcdBuffer1[16]={0};
  char lcdBuffer2[16]={0};
    /*
  [storename]온도@습도@led@lock@humanCount@FANPWM
  -> strtok() 이후로 lcd_update()
  */  
  sprintf(lcdBuffer1, "%2.1f`C led:%d   ", temp, ledState);
  sprintf(lcdBuffer2, "%s h:%d f:%d  ", lockState ? "lock" : "open", humanCount, fanPwm);

  lcd.setCursor(0, 0);
  lcd.print(lcdBuffer1);
  lcd.setCursor(1, 0);
  lcd.print(lcdBuffer2);
}

void timerIsr() {
  doUpdate = true;
}