#include <SoftwareSerial.h>
#include "lgt_LowPower.h"
#include "differential_amplifier.h"
#define GAIN GAIN_1

SoftwareSerial mySerial(2,3); // RX, TX - выводы к радио-модулю

const int SET = 7;

typedef struct { // структура пакета данных
  int id;  
  int network_id;
  String packet;
} Node;

Node this_node = {
  1, // id
  1, // network id
  "" // packet
};


void transmit_data() { // функция отправки пакета данных
  
    String err_code = "E0"; // по умолчанию код ошибки "0" - нет ошибок
    float voltage = get_voltage();
    if (voltage < -3.5 || voltage > 3.5) {
      err_code = "E1";
    }
    this_node.packet = "N" + String(this_node.network_id) + 
          "D" + String(this_node.id) + 
          "V" + String(voltage) + 
          err_code;
    mySerial.print(this_node.packet); //отправка пакета данных через радио-модуль

  //digitalWrite(13, 1);
  //delay(1);
  //digitalWrite(13, 0);

}

float get_voltage() { //считывание напряжения
  int voltage = analogDiffRead(A0, A3, GAIN);
  float finalVolt = voltage / 1000;
  return finalVolt;
}

void hc_sleep(){
  delay(50);
  digitalWrite(SET, 0);
  delay(50);
  mySerial.println("AT+SLEEP");
  delay(100);
  digitalWrite(SET, 1);
}

void hc_wake(){
  delay(100);
  digitalWrite(SET, 0);
  delay(50);
  digitalWrite(SET, 1);
  delay(50);
}

void setup() { 
  Serial.begin(9600);
  mySerial.begin(9600);
  /*
   Установка режимов радио-модуля:
   digitalWrite(SET, 0);
  delay(50);
  mySerial.print("AT+FU1"); // установка режима FU1 (FU2, FU3, FU4 аналогично)
  delay(50);
  digitalWrite(SET, 1);
  delay(50);
  */
  pinMode(13,OUTPUT);
  pinMode(SET, OUTPUT);
  digitalWrite(13,0);
  }


void loop() { 
  hc_sleep();
  LowPower.powerDown(SLEEP_16S, ADC_OFF, BOD_OFF); // установка режима сна на 16 секунд
  hc_wake(); 
  transmit_data(); // отправка пакета данных
 }
