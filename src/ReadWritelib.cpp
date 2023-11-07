#include<Arduino.h>
#include <BLEDevice.h>
#include<ReadWritelib.h>
#include<EEPROM.h>
String Default =  "00:00:00:00";

bool isFirstDevice(){
  String tmp = readStringFromEEPROM(0,11);
  if(tmp == Default) return true;
  else return false;
}
void writeStringToEEPROM(int addr, const String &str) {
  int length = str.length();
  EEPROM.write(addr, length);
  for (int i = 0; i < length; i++) {
    EEPROM.write(addr  + i, str[i]);
  }
  delay(200);
  EEPROM.commit();
}
String readStringFromEEPROM(int addr, int length) {
  String str = "";
  for (int i = 0; i < length; i++) {
    str += char(EEPROM.read(addr + i ));
  }
  return str;
}
std::string intToString(int num) {
    std::string result_String;
    bool isNegative = false;

    if (num < 0) {
        isNegative = true;
        num = -num;
    }

    do {
        result_String = char(num % 10 + '0') + result_String;
        num /= 10;
    } while (num > 0);

    if (isNegative) {
        result_String = "-" + result_String;
    }

    return result_String;
}
std::string convert(std::string input){ //(input)ID=00:00:00:00 -> 00:00:00:00
    int l = input.length() - 3;
    std::string tmp = "";
    for(int i = 0 ; i < l ; i++){
        tmp += input[i+3]; 
    }
    return tmp;
}
void updateStep(BLECharacteristic* Characteristic ){
  if(EEPROM.read(12) > 0){
    std::string msg = "step=" + intToString((int)EEPROM.read(12));
    Characteristic->setValue(msg);
    Characteristic->notify();
    delay(2000);
    Serial.println("gửi số bước chân còn lại lên app");
    EEPROM.write(12,0);
    EEPROM.commit();
  }
}
void disconnect(BLEServer* ConnectedServer){
    delay(3000);
    ConnectedServer->disconnect(ConnectedServer->getConnectedCount());
    ConnectedServer = NULL;
    delay(1000);
}
