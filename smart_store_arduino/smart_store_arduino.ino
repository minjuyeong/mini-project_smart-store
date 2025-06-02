#include <SoftwareSerial.h>
#include <Wire.h>
#include <MsTimer2.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define MSG_SIZE 60
#define pArray_CNT 15
#define DEBUG

char recvId[10] = "MJY_LIN";
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

volatile bool doUpdate = false;
bool waitingForInput = false;

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
}

void loop()
{
  inputvalue = keypad.getKey();

  if (waitingForInput && inputvalue != NO_KEY) {
    storename = '\0';
    stopFlag = 0;
    waitingForInput = false;
    lcd.clear();
    memset(lcdBuffer1, 0, sizeof(lcdBuffer1));
    memset(lcdBuffer2, 0, sizeof(lcdBuffer2));
    return;
  }

  if (inputvalue == 'A' || inputvalue == 'B' || inputvalue == 'C' || inputvalue == 'D') {
    storename = inputvalue;
    send();         // send + recv + lcd_update
    waitingForInput = true;
  }
  else if (storename == '\0' && inputvalue != NO_KEY) {
    keypadInput();
    send();
  }
  else if (doUpdate && waitingForInput) {
    doUpdate = false;
    recv();         // just receive
    lcd_update();  // update LCD from buffer
  }
  else {
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
      if (key == '#') {
        lcd.setCursor(0, 1);
        if (input == password) {
          lcd.print("OK!             ");
          delay(2000);
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

void recv()
{
  int i = 0;
  char *pToken;
  char *pArray[pArray_CNT] = {0};
  char recvBuffer[MSG_SIZE] = {0};
  int len = BTSerial.readBytesUntil('\n', recvBuffer, sizeof(recvBuffer) - 1);

  if (len <= 0) return;

  pToken = strtok(recvBuffer, "[@]");
  while (pToken != NULL) {
    pArray[i++] = pToken;
    if (i >= pArray_CNT) break;
    pToken = strtok(NULL, "[@]");
  }

  if (pArray[2] && pArray[3] && pArray[4] && pArray[5] && pArray[6] && pArray[7]) {
    strcpy(temp, pArray[2]);
    humi = atof(pArray[3]);
    ledState = atoi(pArray[4]);
    lockState = atoi(pArray[5]);
    humanCount = atoi(pArray[6]);
    fanPwm = atoi(pArray[7]);
    sprintf(lcdBuffer1, "%s`C led:%d", temp, ledState);
    sprintf(lcdBuffer2, "%s h:%d f:%d", lockState ? "lock" : "open", humanCount, fanPwm);
  } else if (strcmp(pArray[1], "ALLSTOP") == 0) {
    stopState = atoi(pArray[2]);
    sprintf(lcdBuffer1, "ALL STOP %s", stopState ? "ON" : "OFF");
    lcdBuffer2[0] = '\0';
  }
}

void lcd_update()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(lcdBuffer1);
  lcd.setCursor(0, 1);
  lcd.print(lcdBuffer2);
}

void lcd_clock()
{
  lcd.setCursor(0, 0);
  lcd.print("clock clock     ");
  lcd.setCursor(0, 1);
  lcd.print("                ");
}

void timerIsr()
{
  doUpdate = true;
}