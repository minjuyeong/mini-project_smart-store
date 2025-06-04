#include <SoftwareSerial.h>
#include <Wire.h>
#include <MsTimer2.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define MSG_SIZE 60
#define pArray_CNT 15
#define DEBUG

unsigned long lastKeyTime = 0;

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

String password = "1234";
String input = "";

char storename = '\0';
char inputvalue;
char lcdBuffer1[16] = {0};
char lcdBuffer2[16] = {0};

char temp[5];
float humi;
int lockState;
int stopState;
int ledState;
int humanCount;
int fanPwm;

int stopFlag = 0;

SoftwareSerial BTSerial(10, 11);

volatile bool timerIsrFlag = false;
bool waitingForInput = false;
bool updatTimeFlag = true;

typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
  char week[4];
} DATETIME;

DATETIME dateTime = {0, 0, 0, 12, 0, 0};

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("setup() start!");
#endif
  lcd_init();
  BTSerial.begin(9600);
  MsTimer2::set(1000, timerIsr);
  MsTimer2::start();

  char sendBuffer[MSG_SIZE] = {0};
  sprintf(sendBuffer, "[GETTIME]\n");
  BTSerial.write(sendBuffer, strlen(sendBuffer));
  recv();
  updatTimeFlag = false;
}

void loop()
{
  inputvalue = keypad.getKey();

  if (waitingForInput) {
    char key = keypad.getKey();
    if (key != NO_KEY && millis() - lastKeyTime > 200) {
      lastKeyTime = millis();
      storename = '\0';
      stopFlag = 0;
      waitingForInput = false;
      lcd.clear();
      memset(lcdBuffer1, 0, sizeof(lcdBuffer1));
      memset(lcdBuffer2, 0, sizeof(lcdBuffer2));
    }
    return;
  }

  if (inputvalue == 'A' || inputvalue == 'B' || inputvalue == 'C' || inputvalue == 'D') {
    storename = inputvalue;
    send();
    waitingForInput = true;
  }
  else if (storename == '\0' && inputvalue != NO_KEY) {
    keypadInput();
    send();
  }
  else if (timerIsrFlag && waitingForInput) {
    timerIsrFlag = false;
    recv();
    lcd_update();
  }
  else {
    sprintf(lcdBuffer1, "%02d.%02d.%02d %s", dateTime.year, dateTime.month, dateTime.day, dateTime.week);
    sprintf(lcdBuffer2, "%02d:%02d:%02d", dateTime.hour, dateTime.min, dateTime.sec);

    lcd_update();

    if (updatTimeFlag) {
      char sendBuffer[MSG_SIZE] = {0};
      sprintf(sendBuffer, "[GETTIME]\n");
      BTSerial.write(sendBuffer, strlen(sendBuffer));
      recv();
      updatTimeFlag = false;
    }
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
  input = "";
  storename = 0;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("passward :      ");

  while (1) {
    char key = keypad.getKey();

    if (key != NO_KEY && key != prevKey) {
      prevKey = key;

      if (key == '#') {
        lcd.setCursor(0, 1);
        if (input == password) {
          lcd.print("OK!             ");
          unsigned long okTime = millis();
          while (millis() - okTime < 2000);

          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("select store");

          do {
            storename = keypad.getKey();
          } while (storename == NO_KEY);

          if (storename == 'A' || storename == 'B' || storename == 'C' || storename == 'D') {
            stopFlag = 1;
            sprintf(abc, "%c store", storename);
            lcd.setCursor(0, 1);
            lcd.print(abc);
            return;
          } else {
            lcd.clear();
            lcd.print("reselect store");
            unsigned long errTime = millis();
            while (millis() - errTime < 2000);
            lcd.clear();
            return;
          }

        } else {
          lcd.print("That's Wrong !  ");
          unsigned long wrongTime = millis();
          while (millis() - wrongTime < 2000);
          lcd.clear();
          return;
        }
      } else if (key == '*') {
        input = "";
        lcd.setCursor(0, 1);
        lcd.print("                ");
      } else if (input.length() < 4) {
        input += key;
        lcd.setCursor(input.length() - 1, 1);
        lcd.print(key);
      }
    }

    if (key == NO_KEY) {
      prevKey = NO_KEY;
    }
  }
}

void send()
{
  char sendBuffer[MSG_SIZE] = {0};

  if (stopFlag) {
    sprintf(sendBuffer, "[%c]ALLSTOP\n", storename);
    stopFlag = 0;
  } else {
    sprintf(sendBuffer, "[%c]STATE\n", storename);
  }

#ifdef DEBUG
  Serial.print("Send: ");
  Serial.println(sendBuffer);
#endif

  BTSerial.write(sendBuffer, strlen(sendBuffer));
  recv();
  lcd_update();
}

void 
recv()
{
  int i = 0;
  char *pToken;
  char *pArray[pArray_CNT] = {0};
  char recvBuffer[MSG_SIZE] = {0};

  // 잠깐 대기: 수신 가능할 때까지 기다림 (최대 300ms까지)
  unsigned long startWait = millis();
  while (!BTSerial.available()) {
    if (millis() - startWait > 300) break;  // 최대 300ms 대기
  }
  int len = BTSerial.readBytesUntil('\n', recvBuffer, sizeof(recvBuffer) - 1);

  if (len <= 0) {
  #ifdef DEBUG
    Serial.println("recv() timeout or empty");
  #endif
    return;
  }

  pToken = strtok(recvBuffer, "[@] ");
  while (pToken != NULL) {
    pArray[i++] = pToken;
    if (i >= pArray_CNT) break;
    pToken = strtok(NULL, "[@] ");
  }

  if (!strcmp(pArray[0], "GETTIME")) {
    dateTime.year = (pArray[1][0] - '0') * 10 + pArray[1][1] - '0';
    dateTime.month = (pArray[1][3] - '0') * 10 + pArray[1][4] - '0';
    dateTime.day = (pArray[1][6] - '0') * 10 + pArray[1][7] - '0';
    dateTime.hour = (pArray[1][9] - '0') * 10 + pArray[1][10] - '0';
    dateTime.min = (pArray[1][12] - '0') * 10 + pArray[1][13] - '0';
    dateTime.sec = (pArray[1][15] - '0') * 10 + pArray[1][16] - '0';
    strncpy(dateTime.week, &pArray[1][18], 3);
    dateTime.week[3] = '\0';  // null 종료 필수!

    lcdBuffer1[0] = '\0';
    lcdBuffer2[0] = '\0';
    sprintf(lcdBuffer1, "%02d.%02d.%02d %s", dateTime.year, dateTime.month, dateTime.day, dateTime.week);
    sprintf(lcdBuffer2, "%02d:%02d:%02d", dateTime.hour, dateTime.min, dateTime.sec);
    lcd_update();
  }
  else if (pArray[2] && pArray[3] && pArray[4] && pArray[5] && pArray[6] && pArray[7]) {
    strcpy(temp, pArray[2]);
    humi = atof(pArray[3]);
    ledState = atoi(pArray[4]);
    lockState = atoi(pArray[5]);
    humanCount = atoi(pArray[6]);
    fanPwm = atoi(pArray[7]);
    sprintf(lcdBuffer1, "%s`C led:%d", temp, ledState);
    sprintf(lcdBuffer2, "%s h:%d f:%d", lockState ? "lock" : "open", humanCount, fanPwm);
    lcd_update();
  }
  else if (strcmp(pArray[1], "ALLSTOP") == 0) {
    stopState = atoi(pArray[2]);
    sprintf(lcdBuffer1, "ALL STOP %s", stopState ? "ON" : "OFF");
    lcdBuffer2[0] = '\0';
  }
  else if (pArray[0] == NULL) {
    sprintf(lcdBuffer1, "recv err");
    lcdBuffer2[0] = '\0';
    lcd_update();
    return;
  }
}

void lcd_update()
{
  static char prevLine1[17] = {0};
  static char prevLine2[17] = {0};

  if (strcmp(prevLine1, lcdBuffer1) != 0 || strcmp(prevLine2, lcdBuffer2) != 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(lcdBuffer1);
    lcd.setCursor(0, 1);
    lcd.print(lcdBuffer2);
    strncpy(prevLine1, lcdBuffer1, sizeof(prevLine1));
    strncpy(prevLine2, lcdBuffer2, sizeof(prevLine2));
  }
}

void clock_calc(DATETIME *dt) {
  dt->sec++;
  if (dt->sec >= 60) {
    dt->sec = 0;
    dt->min++;
    if (dt->min >= 60) {
      dt->min = 0;
      dt->hour++;
      if (dt->hour >= 24) {
        dt->hour = 0;
      }
      updatTimeFlag = true;
    }
  }
}

void timerIsr()
{
  timerIsrFlag = true;
  clock_calc(&dateTime);
}
