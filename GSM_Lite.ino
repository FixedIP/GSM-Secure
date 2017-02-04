#include <EEPROM.h>
#include <OneWire.h>
#define zone1 A0
#define Zone1LED 4
#define alarm1LED 5
#define power A3
int statusPower;
byte powerBad;
int statusZ;
int timeOUT;
int timeIN;
int statusO;
int powerTime;
byte alarm;
byte armed = 0;
byte armedIN = 0;
byte balarm = 0;
const byte saveKey = 7; // вход для кнопки обнуления
const byte statusLed = 6;
OneWire  ds(8);

byte addr[8];
byte allKey; // всего ключей

// функция сверяет два адреса (два массива)
boolean addrTest(byte addr1[8], byte addr2[8]){
  for(int i = 0; i < 8; i++) {
    if (addr1[i] != addr2[i]) return 0;
  }
  return 1;
}

void error(){
  while(1){
    digitalWrite(statusLed, !digitalRead(statusLed));
    delay(300);
    digitalWrite(statusLed, !digitalRead(statusLed));
    delay(300);
    digitalWrite(statusLed, !digitalRead(statusLed));
    delay(300);
    break;

  }
}//

boolean keyTest(){ // возвращает 1 если ключ есть в еепром
  byte addrTemp[8];  
  for (int i = 0; i < allKey; i++){  
    for(int y = 0; y < 8; y++) addrTemp[y] = EEPROM.read((i << 3) + y );   
    if (addrTest(addrTemp, addr)) return 1;

  }     
  return 0;
}//

void save(){ // сохраняет ключ в еепром

  digitalWrite(statusLed, HIGH);    
  if (allKey >= 10) error(); // если места нет 

  while (!ds.search(addr)) ds.reset_search(); // ждем ключ 
  if ( OneWire::crc8( addr, 7) != addr[7]) error();
  if (keyTest()) error(); // если нашли ключ в еепром.

  for(int i = 0; i < 8; i++) EEPROM.write((allKey << 3) + i, addr[i]);    
  delay(3);
  allKey++; // прибавляем единицу к количеству ключей 
  EEPROM.write(511, allKey); 
  digitalWrite(statusLed, LOW);
  delay(300);
}



void setup () {
  Serial.begin(9600);
  pinMode(zone1, INPUT);
  pinMode(Zone1LED, OUTPUT);
  pinMode(alarm1LED, OUTPUT);
  pinMode(statusLed, OUTPUT);  
  pinMode(saveKey, INPUT_PULLUP);
  pinMode(power, INPUT_PULLUP);


  // если при включении нажата кнопка, сбрасываем ключи на 0 
  if (!digitalRead(saveKey)) EEPROM.write(511, 0);

  allKey = EEPROM.read(511); // читаем количество ключей
}
void loop () {
  ds.reset_search();
  Power ();
  statusZ = analogRead(zone1);


  if (keyTest()) 
  {
    button(); // если нашли ключ в еепром, 
  }

  if (armed == 0) 
  {
    digitalWrite (Zone1LED, LOW);
    digitalWrite (alarm1LED, LOW);
    alarm = 0;
    timeOUT = 0;
    timeIN = 0;
    armedIN = 0;
    balarm = 0;
  }

  if (armed == 1 && alarm == 0 && armedIN == 0)
  {


    if (timeIN == 200)// 100 - 10сек. 200 - 30сек. 300 - 1мин.
    {
      if (statusZ > 150 && statusZ < 200)
      {
        if (powerBad == 0)
        {
          Serial.println("Norma");
          digitalWrite (Zone1LED, HIGH);
          digitalWrite (alarm1LED, LOW);
          armedIN = 1;
        }
        else 
        {
          digitalWrite (Zone1LED, HIGH);
          delay(2000);
          digitalWrite (alarm1LED, LOW);
          digitalWrite (Zone1LED, LOW);
          armedIN = 1;
        }
      }
      else armed = 0;
    }
    if (armedIN ==0)
    {
      timeIN++;
      if (powerBad == 0)
      {
        Serial.print("timeIN = ");
        Serial.println(timeIN);
        delay(timeIN);
        digitalWrite (Zone1LED, HIGH);
        delay(10);
        digitalWrite (Zone1LED, LOW);
        delay(10);
        digitalWrite (Zone1LED, HIGH);
        delay(10);
        digitalWrite (Zone1LED, LOW);
      }
      else
      {
        delay(timeIN);
        digitalWrite (Zone1LED, HIGH);
        delay(3);
        digitalWrite (Zone1LED, LOW);
        delay(3);
        digitalWrite (Zone1LED, HIGH);
        delay(3);
        digitalWrite (Zone1LED, LOW);
      }
    }


  }
  if (armed == 1 && alarm == 0)
  {
    if (balarm == 0 && armedIN == 1)
    {
      if (statusZ < 150 || statusZ > 200 )
      {
        balarm = 1;
      }
    }
    if (timeOUT == 300)
    {
      if (powerBad == 0)
      {
        Serial.println("Alarm");
        digitalWrite (Zone1LED, LOW);
        digitalWrite (alarm1LED, HIGH);
        //digitalWrite (STOP1LED, LOW);
        alarm = 1;
        timeOUT = 0;
        balarm = 0;
      }
      else
      {
        digitalWrite (Zone1LED, LOW);
        digitalWrite (alarm1LED, HIGH);
        delay(2000);
        digitalWrite (alarm1LED, LOW);
        //digitalWrite (STOP1LED, LOW);
        alarm = 1;
        timeOUT = 0;
        balarm = 0;
      }
    }
    if (balarm == 1 && armedIN == 1)
    {
      timeOUT++;
      if (powerBad == 0)
      {
        Serial.print("timeOUT = ");
        Serial.println(timeOUT);
        delay(timeOUT);
        digitalWrite (alarm1LED, HIGH);
        delay(10);
        digitalWrite (alarm1LED, LOW);
        delay(10);
        digitalWrite (alarm1LED, HIGH);
        delay(10);
        digitalWrite (alarm1LED, LOW);
      }
      else
      {
        delay(timeOUT);
        digitalWrite (alarm1LED, HIGH);
        delay(3);
        digitalWrite (alarm1LED, LOW);
        delay(3);
        digitalWrite (alarm1LED, HIGH);
        delay(3);
        digitalWrite (alarm1LED, LOW);
      }
    }
  }

  if (!digitalRead(saveKey) && armed == 0) save(); // если нажали кнопку  
  // сканируем шину, если нет устройств выходим из loop
  if (!ds.search(addr)) return; 
  if ( OneWire::crc8( addr, 7) != addr[7]) return;
}
void button ()
{
  armed = !armed;
  for(int i = 0; i < 8; i++) {
    addr[i] = 0;
  }

  delay(2000);
}
void Power ()
{
  statusPower = analogRead(power);
  if (statusPower >= 1010)
  {
    //Serial.println ("Power OK ");
    powerBad = 0;
    if (armed == 1 && alarm == 0 && digitalRead(Zone1LED) == LOW)
    {
      digitalWrite (Zone1LED, HIGH);
    }
    if (armed == 1 && alarm == 1 && digitalRead(alarm1LED) == LOW)
    {
      digitalWrite (alarm1LED, HIGH);
    }
  }
  else 
  {
    // Serial.println ("Power BAD!!! ");
    powerBad = 1;
    if (powerTime == 10000)// 10000 - 17 сек. ????
    {
      digitalWrite(alarm1LED, HIGH);
      delay(3);
      digitalWrite(alarm1LED, LOW);
      powerTime = 0;
    }
    powerTime ++;
    if (armed == 1 && digitalRead(Zone1LED) == HIGH)
    {
      digitalWrite (Zone1LED, LOW);
    }
    if (armed == 1 && alarm == 1 && digitalRead(alarm1LED) == HIGH)
    {
      digitalWrite (alarm1LED, LOW);
    }
  }
}














