/*
  blue test: 
  http://www.kccistc.net/
  작성일 : 2024.03.22
  작성자 : IoT 임베디드 KSH
*/
#include <SoftwareSerial.h>
#include <Wire.h>
#include <DHT.h>
#include <MsTimer2.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

#define DHTTYPE DHT11
#define ARR_CNT 5
#define CMD_SIZE 60
char lcdLine1[17] = "Smart IoT By KJH";
char lcdLine2[17] = "";
char sendBuf[CMD_SIZE];
char recvId[10] = "KJH_SQL";  // SQL 저장 클라이이언트 ID

bool timerIsrFlag = false;
bool cdsFlag = false;
int getSensorTime;
SoftwareSerial BTSerial(10, 11); // RX ==>BT:TXD, TX ==> BT:RXD
void setup()
{
  lcd.init();
  lcd.backlight();
  lcdDisplay(0, 0, lcdLine1);
  lcdDisplay(0, 1, lcdLine2);
 
  BTSerial.begin(9600); // set the data rate for the SoftwareSerial port
  MsTimer2::set(1000, timerIsr); // 1000ms period
  MsTimer2::start();
  dht.begin();
}

void loop()
{
  if (BTSerial.available())
    bluetoothEvent();

  if (timerIsrFlag)
  {
    timerIsrFlag = false;
    }    
    
  }

}
void bluetoothEvent()
{
  int i = 0;
  char * pToken;
  char * pArray[ARR_CNT] = {0};
  char recvBuf[CMD_SIZE] = {0};
  int len = BTSerial.readBytesUntil('\n', recvBuf, sizeof(recvBuf) - 1);

  pToken = strtok(recvBuf, "[@]");
  while (pToken != NULL)
  {
    pArray[i] =  pToken;
    if (++i >= ARR_CNT)
      break;
    pToken = strtok(NULL, "[@]");
  }

   if (!strcmp(pArray[0], "KJH_ARD")) { //자기자신을 순환 반복 메시지 방지.
    return ;
   }
  if (!strcmp(pArray[1], "LAMP")) {
  }
}
void timerIsr()
{
  timerIsrFlag = true;
  secCount++;
}
void lcdDisplay(int x, int y, char * str)
{
  int len = 16 - strlen(str);
  lcd.setCursor(x, y);
  lcd.print(str);
  for (int i = len; i > 0; i--)
    lcd.write(' ');
}