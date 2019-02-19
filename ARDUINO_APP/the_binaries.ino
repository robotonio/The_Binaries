#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h> //for LCD I2C

/* Bluetooth configuration*/
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

/*
 * usefull variables
*/
// max soft grabber's pressure in  
int maxPressure = 25; //calibrate it! (< 100)
int modeNumber = 1; //mode = 1 in setup
int previousMode = 1; //keep previous mode to know new change
//the max and min servo angle for hard actuator (calibrate it!)
int servoOpenDegrees = 180;//close
int servoCloseDegrees = 50;//open
//the distance threshold than actuate soft grabber in cm (calibrate it!)
int actDistanceSoft = 10;
//the distance threshold than actuate hard grabber in cm (calibrate it!)
int actDistanceHard = 10;
//flag for soft actuator (open/closed)
boolean softIsOpen = true;
//flag for hard actuator (open/closed)
boolean hardIsOpen = true;
long lastBTms = 0;//last communication with bluetooth 
long btSlot = 1000; //send bluetooth data every btSlot milliseconds
char BT_data[23];//buffer to keep data for bluetooth devices
long interruptTime = 0;
boolean interruptFlag = true;

void releaseSoft();
void releaseHard();
void grabSoft();
void grabHard();
bool is_number(const std::string& s);
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }

        Serial.println();

        // Do stuff based on the command received from the app
        if (rxValue.find("S0") != -1) { 
          Serial.print("Soft Release");
          releaseSoft();
        } else if (rxValue.find("S1") != -1) { 
          Serial.print("Soft Grab");
          grabSoft();
        } else if (rxValue.find("H0") != -1) { 
          Serial.print("Hard Release");
          releaseHard();
        } else if (rxValue.find("H1") != -1) { 
          Serial.print("Hard Grab");
          grabHard();
        } else {
          Serial.print("New pressure");
          int newMaxPressure = atoi(rxValue.c_str());//std::stoi(rxValue);
          if(newMaxPressure > 0 && newMaxPressure < 100){
            maxPressure = newMaxPressure;
          }
        }

        Serial.println();
        Serial.println("*********");
      }
    }
};


LiquidCrystal_I2C lcd(0x3f, 16, 2);
Servo servo;

/*
 * Modes:
 *  1: on / off buttons (same for soft & hard gribbers)
 *  2: ultrasonic sensor use
 *  3: bluetooth use
*/

/*
 * All the pins
*/
// power button
int pinLedPower = 25;
// on-off buttons mode LED
int pinLedMode1 = 12;
// ultrasonic mode LED
int pinLedMode2 = 14;
// bluetooth mode LED
int pinLedMode3 = 4;
// change mode button
int pinModeBtn = 17;
// pressure configuration pot
int pinPressurePot = 35;
// change max pressure button
int pinPressureButton = 13;
// open grippers button
int pinOpenButton = 27;
// close grippers button
int pinCloseButton = 16;
// pressure sensor
int pinPressureSensor = 34;
// servo motor
int pinServo = 26;
// solenoid inflate gripper
int pinSolenoidInflate = 5;
// solenoid expansion gripper
int pinSolenoidExpanse = 18;
// dc motor (to close soft gripper)
int pinMotor = 19;
// Ultrasonic for soft gripper
int pinTrigHcSoft = 36;//2;
int pinEchoHcSoft = 39;//33;
// Ultrasonic for hard gripper
int pinTrigHcHard = 2;//36;
int pinEchoHcHard = 33;//39;
// Ultrasonic for 2nd soft gripper
int pinTrigHcSoft2 = 32;
int pinEchoHcSoft2 = 15;



/*
 * This function is triggered when a new
 * interrupt happens. In this way we change
 * the mode, using the mode button
*/
void handleModeInterrupt(){
  //move modeNumber to next mode
  interruptTime = millis();
  if(interruptFlag){
    modeNumber++;
    //max modeNumber is 2
    if(modeNumber > 2){
      modeNumber = 1;
    }
  }
}

void setup() {
  Serial.begin(115200);
  //configure servo for hard actuator
  servo.attach(pinServo);
  servo.write(servoOpenDegrees);//in setup hard actuator is closed
  //configure LCD
  lcd.init();
  lcd.backlight();
  //configure soft Ultrasonic Sensor
  pinMode(pinTrigHcSoft, OUTPUT); // Sets the trigPin as an Output
  pinMode(pinEchoHcSoft, INPUT); // Sets the echoPin as an Input
  //configure soft Ultrasonic Sensor
  pinMode(pinTrigHcSoft2, OUTPUT); // Sets the trigPin as an Output
  pinMode(pinEchoHcSoft2, INPUT); // Sets the echoPin as an Input
  //configure hard Ultrasonic Sensor
  pinMode(pinTrigHcHard, OUTPUT); // Sets the trigPin as an Output
  pinMode(pinEchoHcHard, INPUT); // Sets the echoPin as an Input
  //configure LEDs
  pinMode(pinLedPower, OUTPUT);
  pinMode(pinLedMode1, OUTPUT);
  pinMode(pinLedMode2, OUTPUT);
  pinMode(pinLedMode3, OUTPUT);
  //configure buttons
  pinMode(pinModeBtn, INPUT_PULLUP);
  pinMode(pinOpenButton, INPUT_PULLUP);
  pinMode(pinCloseButton, INPUT_PULLUP);
  pinMode(pinPressureButton, INPUT_PULLUP);
  //configure motors
  pinMode(pinSolenoidInflate, OUTPUT);
  pinMode(pinSolenoidExpanse, OUTPUT);
  pinMode(pinMotor, OUTPUT);

  //initialise actuators (all closed)
  digitalWrite(pinSolenoidInflate, LOW);
  digitalWrite(pinSolenoidExpanse, LOW);
  digitalWrite(pinMotor, LOW);

  //initialise LEDs (in setup mode=1)
  digitalWrite(pinLedPower, HIGH);//never changes
  digitalWrite(pinLedMode1, HIGH);
  digitalWrite(pinLedMode2, LOW);
  digitalWrite(pinLedMode3, LOW);
  setMode(modeNumber);
  //configure interrupt to change mode
  attachInterrupt(digitalPinToInterrupt(pinModeBtn), handleModeInterrupt, FALLING);

  /* Bluetooth set up*/
  // Create the BLE Device
  BLEDevice::init("THE_BINARIES"); // Give it a name

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();

  /* End bluetooth configuration*/
}

void loop() {
  //check id modeNumber changed to update the system mode
  if(millis() - interruptTime > 1000){
    interruptFlag = true;
  }
  if(previousMode != modeNumber){
    digitalWrite(pinSolenoidExpanse, HIGH);
    interruptFlag = false;
    setMode(modeNumber);
    previousMode = modeNumber;//update previousMode
  }
  //mode options
  if(modeNumber == 1){//move actuators with buttons
    runMode1();
  } else if(modeNumber == 2){//grab with sensor and free with button
    runMode2();
  }
  //check pressure button to change max pressure
  if(digitalRead(pinPressureButton) == LOW){
    Serial.println("pinPressureButton pressed");
    controlMaxPressure();    
  }

  if(millis() - lastBTms > btSlot){
    bluetoothApp();
    lastBTms = millis();
  }
}

/*
 * Control both actuators with the 
 * grab&free buttons
*/
void runMode1(){
  if(digitalRead(pinOpenButton) == LOW){
    long openTime = millis();
    boolean pressureMode = false;
    while(digitalRead(pinOpenButton) == LOW){
      if(millis() - openTime > 3000){
        pressureMode = true;
        break;
      } 
    }
    if(pressureMode){
      controlMaxPressure();
    } else {
      Serial.println("open button");
      releaseSoft();
      releaseHard();
      delay(500); 
    }
  }
  if(digitalRead(pinCloseButton) == LOW){
    Serial.println("close button");
    grabSoft();
    grabHard();
    delay(500);
  }
}


/*
 * Control actuators with ultrasonic sensors
 * The actuator will grab when it's distance 
 * sensor detect an object and will free thw object
 * when user push the release button 
*/
void runMode2(){
  //grab when detect object AND actuator is open
  int softDistance = readDistance(1);
  int hardDistance = readDistance(2);
  int softDistance2 = readDistance(3);
  Serial.print("Soft distance 1: ");
  Serial.println(softDistance);
  Serial.print("Hard distance: ");
  Serial.println(hardDistance);
  Serial.print("Soft distance 2: ");
  Serial.println(softDistance2);
  if((softDistance < actDistanceSoft || softDistance2 < actDistanceSoft) && softIsOpen){
    grabSoft();
  }
  //grab when detect object AND actuator is open
  if(hardDistance < actDistanceHard && hardIsOpen){
    grabHard();
  }
  //release object after push the release button
  if(digitalRead(pinOpenButton) == LOW){
    //protect actuator if is already open
    if(!softIsOpen){
      releaseSoft();  
    }
    //protect actuator if is already open
    if(!hardIsOpen){
      releaseHard();  
    }
  }
}

void bluetoothApp(){
  if (deviceConnected) {
    digitalWrite(pinLedMode3, HIGH);
    updateData();
    Serial.print("*** Sent Value: ");
    Serial.print(BT_data);
    Serial.println(" ***");
    
    pCharacteristic->setValue(BT_data);
    pCharacteristic->notify(); // Send the value to the app!
  } else {
    digitalWrite(pinLedMode3, LOW);
  }
}

/*
 * Control the max pressure of the closed soft
 * grabber, with the pot
*/
void controlMaxPressure(){
  // warn user with LCD message for 3 seconds
  for(int i = 0; i < 3; i++){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("CHANGE PRESSURE");
    delay(1000);
  }
  //old max pressure at the first line of LCD
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Current: ");
  lcd.setCursor(10,0);
  lcd.print(maxPressure);
  lcd.setCursor(0,1);
  //pot value at the first line of LCD, until push pinPressureButton
  lcd.print("New: ");
  while(digitalRead(pinOpenButton) == HIGH){
    lcd.setCursor(6,1);
    lcd.print("    ");
    lcd.setCursor(6,1);
    lcd.print((analogRead(pinPressurePot)*100)/4095);
    delay(300);
  }
  //change max pressure
  maxPressure = (analogRead(pinPressurePot)*100)/4095;
  // restore mode
  setMode(modeNumber);
  delay(1000);
}

/*
 * For soft grabber, gr == 1 
 * For hard grabber, gr == 2
 */
int readDistance(byte gr){
  int tr = 0;
  int ech = 0;
  long duration;
  //use the right ultrasonic pins
  if(gr == 1){
    tr = pinTrigHcSoft;
    ech = pinEchoHcSoft;
  }
  if(gr == 2){
    tr = pinTrigHcHard;
    ech = pinEchoHcHard;
  }
  if(gr == 3){
    tr = pinTrigHcSoft2;
    ech = pinEchoHcSoft2;
  }
  // Clears the trigPin
  digitalWrite(tr, LOW);
  delayMicroseconds(2);
  
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(tr, HIGH);
  delayMicroseconds(10);
  digitalWrite(tr, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ech, HIGH);
  
  // return the distance in CMs
  return duration*0.034/2;
}

/*
 * Simple returns the soft actuator's pressure
 * in the range of 0-4095
*/
int readPressure(){
  return (analogRead(pinPressureSensor)*100)/4095;//
}

/*
 * Grab an object with the soft actuator
*/
void grabSoft(){
  Serial.println("grab soft - start");
  //protect the grabber if is already closed
  if(!softIsOpen){
    return;
  }
  //update LCD info
  lcd.setCursor(0, 1);
  lcd.print("               ");
  lcd.setCursor(0, 1);
  lcd.print("CLOSE:");
  //open inflate solinoid
  digitalWrite(pinSolenoidInflate, HIGH);
  //open expanse solinoid
  digitalWrite(pinSolenoidExpanse, LOW);
  //actuate motor to inflate soft grabber
  digitalWrite(pinMotor, HIGH);
  //inflate until air pressure is fine or until press open button (for safe)
  while(readPressure() < maxPressure && digitalRead(pinOpenButton) == HIGH){
    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.setCursor(8, 1);
    lcd.print(readPressure());
    delay(50);
  }
  digitalWrite(pinSolenoidInflate, LOW);
  digitalWrite(pinMotor, LOW);
  lcd.setCursor(8, 1);
  lcd.print("     ");
  lcd.setCursor(8, 1);
  lcd.print(readPressure());
  //stop inflation
  
  //change soft grabber status to closed
  softIsOpen = false;
  Serial.println("grab soft - end");
}

void releaseSoft(){
  Serial.println("release soft - start");
  //protect the grabber if is already closed
  if(softIsOpen){
    return;
  }
  //update LCD info
  lcd.setCursor(0, 1);
  lcd.print("               ");
  lcd.setCursor(0, 1);
  lcd.print("SOFT OPEN");
  digitalWrite(pinSolenoidExpanse, HIGH);
  delay(2000);
  digitalWrite(pinSolenoidExpanse, LOW);
  lcd.setCursor(0, 1);
  lcd.print("               ");
  softIsOpen = true;
  Serial.println("release soft - end");
}

void grabHard(){
  Serial.println("grab hard");
  //protect the grabber if is already closed
  if(!hardIsOpen){
    return;
  }
  //update LCD info
  lcd.setCursor(0, 1);
  lcd.print("               ");
  lcd.setCursor(0, 1);
  lcd.print("HARD CLOSE");
  //close hard grabber slowly
  for(int i = servoOpenDegrees; i > servoCloseDegrees; i--){
    servo.write(i);
    delay(10);//change it if it's too slow or fast!
  }
  //change hard grabber status to closed
  hardIsOpen = false;
  Serial.println("grab hard - end");
}

void releaseHard(){
  Serial.println("release hard");
  if(hardIsOpen){
    return;
  }
  //update LCD info
  lcd.setCursor(0, 1);
  lcd.print("               ");
  lcd.setCursor(0, 1);
  lcd.print("HARD OPEN");
  for(int i = servoCloseDegrees; i < servoOpenDegrees; i++){
    servo.write(i);
    delay(10);
  }
  lcd.setCursor(0, 1);
  lcd.print("               ");
  hardIsOpen = true;
  Serial.println("release hard - end");
}

void setMode(int m){
  Serial.print("New Mode:");
  Serial.println(m);
  String msg = "";
  modeNumber = m;
  lcd.clear();
  lcd.setCursor(0, 0);
  if(modeNumber == 1){
    digitalWrite(pinLedMode1, HIGH);
    digitalWrite(pinLedMode2, LOW);
    digitalWrite(pinLedMode3, LOW);
    lcd.print("Mode: MANUAL");
  } else if(modeNumber == 2){
    digitalWrite(pinLedMode1, LOW);
    digitalWrite(pinLedMode2, HIGH);
    digitalWrite(pinLedMode3, LOW);
    lcd.print("Mode: AUTO");
  } 
  delay(1000);
}

/* update the char array BT_data to send it to bluetooth device*/
void updateData(){
    
    String str;
    //1. mode number
    str = String(modeNumber);
    int index = 0;
    for(int i = 0; i < str.length(); i++){
      BT_data[index++] = str[i];
    }
    BT_data[index++] = ' ';
    //2. soft actuator status
    if(softIsOpen){
      BT_data[index++] = '0';
    } else {
      BT_data[index++] = '1';
    }
    BT_data[index++] = ' ';
    //3. hard actuator status
    if(hardIsOpen){
      BT_data[index++] = '0';
    } else {
      BT_data[index++] = '1';
    }
    BT_data[index++] = ' ';
    //4. max pressure
    str = String(maxPressure);
    for(int i = 0; i < str.length(); i++){
      BT_data[index++] = str[i];
    }
    BT_data[index++] = ' ';
    //5. current pressure
    str = String(readPressure());
    for(int i = 0; i < str.length(); i++){
      BT_data[index++] = str[i];
    }
    BT_data[index++] = ' ';
    //6. soft actuator distance (get min)
    int minDistance = readDistance(1);
    if(readDistance(3) < minDistance){
      minDistance = readDistance(3);
    }
    str = String(minDistance);
    for(int i = 0; i < str.length(); i++){
      BT_data[index++] = str[i];
    }
    BT_data[index++] = ' ';
    //7. hard actuator distance
    str = String(readDistance(2));
    for(int i = 0; i < str.length(); i++){
      BT_data[index++] = str[i];
    }
    BT_data[index] = '\0';
}

bool is_number(const std::string& s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}
