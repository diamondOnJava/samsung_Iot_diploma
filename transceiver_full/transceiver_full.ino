#include <SoftwareSerial.h>
#include "lgt_LowPower.h"
#include "differential_amplifier.h"
#define GAIN GAIN_1 // коэффициенты дифф. усиления
// #define GAIN GAIN_8
// #define GAIN GAIN_16
// #define GAIN GAIN_32

SoftwareSerial mySerial(2,3); // RX, TX - выводы к радио-модулю

const int SET = 7; // вывод для настройки параметров радио-модуля

typedef struct { // структура пакета данных
  int id;  
  int network_id;
  String packet;
} Node;

Node this_node = {
  2, // id
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
  pinMode(13,OUTPUT);
  pinMode(SET, OUTPUT);
  digitalWrite(13,0);
  //attachInterrupt(digitalPinToInterrupt(2), inter, RISING); // прерывание на RX
  }

void repeat() {
  if (mySerial.available()) { // при выходе из сна обработка поступившего пакета
        String transit_packet = mySerial.readString();
  
        int network_id = transit_packet.substring(transit_packet.indexOf('N') + 1, transit_packet.indexOf('D')).toInt(); 
        int device_id = transit_packet.substring(transit_packet.lastIndexOf('D') + 1, transit_packet.lastIndexOf('V')).toInt(); 

        if ((network_id == this_node.network_id) && (device_id < this_node.id)) {
          //Serial.println("Пакет проверен и отправлен");
          mySerial.print(transit_packet); // при успешной проверке пакета он отправляется к след. устр-ву
          transit_packet = "";
        }
  }
}

void loop() { 
  hc_sleep();
  LowPower.powerDown(SLEEP_16S, ADC_OFF, BOD_OFF); // установка режима сна на 16 секунд
  hc_wake();
  repeat(); // ретрансляция
  transmit_data(); // отправка собственного пакета
}
