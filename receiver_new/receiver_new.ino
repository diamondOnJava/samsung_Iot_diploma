#include <ModbusSerial.h>
#include <SoftwareSerial.h>
#include <avr/sleep.h>
#include <PMU.h>

SoftwareSerial mySerial(4,2); // RX, TX
const int SET = 3;
bool flag = false;

String def_network_id = "N1";
const int txPin = 9; // вывод к телеметрии по RS-485
const int slaveId = 1; // идентификатор ведомого устройства
const int device_reg = 0; // регистр хранения id устр-ва
const int voltage_reg = 1; // регистр хранения показаний
const int error_reg = 2; // регистр хранения кода ошибки
ModbusSerial mb(Serial, slaveId, txPin); // объект для вывода показаний в телеметрию

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  pinMode(13,OUTPUT);
  digitalWrite(13,0);
  digitalWrite(SET, 0);
  delay(200);
  mySerial.println("AT+DEFAULT");
  delay(100);
  digitalWrite(SET, 1);
  delay(200);

  mb.config(9600); // конфигурация объекта RS-485
  mb.addHreg(device_reg);
  mb.addHreg(voltage_reg);
  mb.addHreg(error_reg);
  }

String packet = "";

void loop() {
 mb.task(); // обработка команд от ведущего устр-ва 
 if (mySerial.available()) {
        packet = mySerial.readString();
        String network_id = packet.substring(packet.indexOf('N'), packet.indexOf('D')); 
        int device_id = packet.substring(packet.lastIndexOf('D') + 1, packet.lastIndexOf('V')).toInt(); 
        float voltage = packet.substring(packet.lastIndexOf('V') + 1, packet.lastIndexOf('E')).toFloat(); 
        int error_code = packet.substring(packet.lastIndexOf('E') + 1, packet.lastIndexOf('E') + 2).toInt();
        voltage = voltage * 100; 
        if (network_id == def_network_id) {
         Serial.println(device_id);
         Serial.println(voltage);
         Serial.println(error_code);

         mb.setHreg(device_reg, device_id); // запись в регистры хранения
         mb.setHreg(voltage_reg, voltage);
         mb.setHreg(error_reg, error_code);
         voltage = 0;
        }
    }
}
