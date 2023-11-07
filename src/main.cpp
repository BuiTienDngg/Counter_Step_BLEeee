
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <EEPROM.h>
#include <string.h>
#include <ReadWritelib.h>
BLEServer* pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLEServer* pConnectedServer = NULL; 
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define EEPROM_SIZE 15
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool tmpConnected = false;
bool trueConnected = false;
bool isReset = false;
String reset = "RESET";
std:: string receiveData = "" , receiveID = "";
String ID = "00:00:00:15";
RTC_DATA_ATTR int isSleep = 0; 
int length = 14;
int button = 4;
void IRAM_ATTR IRS(){
  delay(500);
  isSleep ++;
}
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      tmpConnected = true; // khi này app vừa pair với thiết bị 
      }
    void onDisconnect(BLEServer* pServer) {
      trueConnected = false;
      tmpConnected = false;
    }
};
class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      if(trueConnected == false){
        std::string tmp = pCharacteristic->getValue();
        receiveID = convert(tmp);
          if (receiveID.length() > 0) {
            Serial.print("New ID value: ");
            for (int i = 0; i < receiveID.length(); i++){
              Serial.print(receiveID[i]);
             }Serial.println();
          }
      }else{
          receiveData = pCharacteristic->getValue();
          if (receiveData.length() > 0) {
              Serial.print("New data value: ");
              for (int i = 0; i < receiveData.length(); i++){
                Serial.print(receiveData[i]);
              }
              Serial.println();
          }
      }
      delay(1000);
    }
};
void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);
  pinMode(button,INPUT_PULLDOWN);
  Serial.println("Starting BLE work!");
  BLEDevice::init("THM_Dũngg");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  // writeStringToEEPROM(0,Default);
  // EEPROM.write(12,0);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_2, 1);
  // esp_wake_deep_sleep();
  attachInterrupt(2,IRS,FALLING);
  // ++isSleep;
  Serial.println("Boot number: " + String(isSleep));
  Serial.print("EEPROM hiện tại là ");
  Serial.print(readStringFromEEPROM(0,11));Serial.print("    ");
  Serial.print(EEPROM.read(12));
  Serial.println();
  Serial.println("Characteristic defined! Now you can read it in your phone!");

}
void loop() {
  if(isSleep % 2 == 1){ // ngủ
    Serial.println("Going to sleep now");
    // BLEDevice::stopAdvertising();
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
  }else { // thức
      esp_wake_deep_sleep();
    Serial.println("wake up");
    if(tmpConnected  && !trueConnected){
      if(isFirstDevice()){
              if(receiveID.length() > 0){
                    writeStringToEEPROM(0,receiveID.c_str());
                    tmpConnected = false;
                    trueConnected = true;
                    pCharacteristic->setValue("CONNECT OK");
                    pCharacteristic->notify();
                    updateStep(pCharacteristic);
              } 
      }else{
              delay(1000);
              if(receiveID.length() > 0){
                      String tmp = receiveID.c_str();
                      if( tmp == readStringFromEEPROM(0,11)){ 
                            Serial.print(tmp);Serial.print("   ");
                            Serial.println(readStringFromEEPROM(0,11));
                            delay(1000);
                            trueConnected = true;
                            tmpConnected = false;
                            pCharacteristic->setValue("CONNECT OK");
                            pCharacteristic->notify();
                            updateStep(pCharacteristic);
                            delay(1000);
                      }else{
                            pCharacteristic->setValue("CONNECT FAIL");
                            pCharacteristic->notify();
                            disconnect(pConnectedServer);
                      } 
              }
      }
  }else if(!trueConnected && !tmpConnected){
              if(digitalRead(button) == 1){
                delay(50);
                while(digitalRead(button) == 1);
                int tmp = EEPROM.read(12);
                tmp++;
                EEPROM.write(12,tmp);
                EEPROM.commit();
                Serial.print("giá trị mới là:   ");
                Serial.println(EEPROM.read(12));
            }
  } else {
              if(digitalRead(button) == 1){
                  delay(50);
                  while(digitalRead(button) == 1);
                  std::string s = "step=1";
                  pCharacteristic->setValue(s);
                  pCharacteristic->notify();
              }
              if((String)receiveData.c_str() == reset){
                  writeStringToEEPROM(0,"00:00:00:00");
                  Serial.println("RESET");
                  //ngắt kết nối
                  disconnect(pConnectedServer);
              }
    }
  }
  
}