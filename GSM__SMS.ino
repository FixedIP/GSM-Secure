/// GSM сигналка c установкой по звонку
/// датчиком на прерывателях

// 

#include <EEPROM.h>

//// как подключен модем?
#include <SoftwareSerial.h>                 // если программный
SoftwareSerial gsm(7, 8); // RX, TX
//#define gsm Serial                           // если аппаратный в UNO
//#define gsm Serial1                          // если аппаратный в леонардо

#define LED 13

#define TELLNUMBER "ATD+7;"                 // номен на который будем звонить
#define SMSNUMBER "AT+CMGS=\"7\""           // номер на который будем отправлять SMS
#define TELMODE "7"                         // номер для установки на охрану

#define SH1 A2              // шлейыф
//#define SH2 A3
#define pinPOWER 4           //Наличие основного питания
#define pinBATTERY A4        //Контроль напряжения резервного питания (аккамуляторов)
#define pinBOOT 5           // нога BOOT или K на модеме 
byte sms = 0;               //
byte mode = 0;              // 0 - только включили
// 1 - установлена охрана
// 2 - снята с охраны
// при добавлении не забываем посмотреть на 41 строку



void setup() {
  delay(1000);                    //// !! чтобы нечего не повисало при включении

  gsm.begin(9600);                         /// незабываем указать скорость работы UART модема
  //  Serial.begin(9600);
  pinMode(pinPOWER,INPUT);
  pinMode(pinBATTERY, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  pinMode(pinBOOT, OUTPUT);                /// нога BOOT на модеме
  pinMode(SH1, INPUT_PULLUP);              /// нога на растяжку
  //  pinMode(SH2, INPUT_PULLUP);              /// нога на растяжку
  gsm.println("AT+CMGF=1");  
  delay(100);
  gsm.println("AT+CSCS=\"GSM\"");   
  delay(100);
  // читаем режим из еепром
  mode = EEPROM.read(0);
  if (mode > 2) mode = 2;                 // проверяем значение в еепром 
  // занимаемся модемом        
  delay(1000);                            
  digitalWrite(LED, HIGH);                // на время включаем лед  
  digitalWrite(pinBOOT, LOW);             /// включаем модем 

  // нужно дождатся включения модема и соединения с сетью
  delay(2000);    
  //  while(gsm.find("STARTUP"));          /// ждем команды от модема  
  gsm.println("ATE0");                  // выключаем эхо  

  while(1){                             // ждем подключение модема к сети
    gsm.println("AT+COPS?");
    if (gsm.find("+COPS: 0")) break;
    digitalWrite(LED, LOW);               // блымаем светодиодом
    delay(50);  
    digitalWrite(LED, HIGH);  
    delay(500); 
  }

  //Serial.println("Modem OK"); 
  digitalWrite(LED, LOW);               // блымаем светодиодом
  delay(1500);  
  digitalWrite(LED, HIGH);
  delay(250); 
  digitalWrite(LED, LOW);   

}

void loop() {
  if(digitalRead(pinPOWER)== LOW){     // если нажали кнопку
    if (sms == 2){
      while(1){             // проверяем готовность модема
        gsm.println("AT+CPAS");
        if (gsm.find("0")) break;
        delay(100);  
      }
      gsm.println (SMSNUMBER); // даем команду на отправку смс
      delay(100);
      gsm.print("POWER Down");  // отправляем текст
      //gsm.print(analogRead(A0));    // и переменную со значением
      gsm.print((char)26);          // символ завершающий передачу
      // Serial.println("ok");
      if (gsm.find("OK")){
        sms++;
      }
    }
  }

  if (mode == 1){                         // если в режиме охраны
    // проверяем датчики
    if (digitalRead(SH1)){                // если обрыв
      // отзваниваемся
      gsm.println(TELLNUMBER); 
      delay(2500);                       
      if (gsm.find("NO CARRIER")){      // ищим сброс вызова,  
        // снимаем охранку
        mode = 2;
        EEPROM.write(0, mode);
      }
    }
  }


  // если охрана снята
  if (mode == 2){
    if (digitalRead(SH1)){  // проверяем датчики, включаем LED
      digitalWrite(LED, HIGH);    
    }
    else digitalWrite(LED, LOW);
  }


  // ищим RING
  // если нашли, опрашиваем кто это и ставим на охрану
  if(gsm.find("RING")){                    // если нашли RING
    gsm.println("AT+CLIP=1");              // включаем АОН, 

    while(1){                              // в цикле
      if (gsm.find(TELMODE)){               // ищим номер телефона, если нашли
        mode = 1;                           // меняем режим  
        EEPROM.write(0, mode);              // пишим его в еепром
        break;                              // и выходим
      }  
      else{                                 // иначе 
        gsm.println("AT+CPAS");             // спрашиваем состояние модема   
        delay(100);
        if (gsm.find("+CPAS: 0")) break;    // и если он в "готовности", выходим из цикла
      }                                     // если звонок в процессе, возвращает +CPAS: 3
    }                                      // и крутимся дальше

    gsm.println("AT+CLIP=0");            // выключаем АОН, 
    delay(500);
    gsm.println("ATH0");                 // сбрасываем вызов 

    digitalWrite(LED, LOW);             // сигнализируем об этом
    delay(500);  
    digitalWrite(LED, HIGH);
    delay(250); 
    digitalWrite(LED, LOW);   

  } 

}


