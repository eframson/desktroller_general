#define CMD_SIZE 128
#define CMD_DELIMITER "|"
#include <string.h>

char receivedBytes[CMD_SIZE];
//const char* CMD_DELIMITER = "|";
int iterator = 0;
int commandTerminator = 10;
int targetDeviceMemSize;
byte targetDeviceMemToRead;
//DESK PCB PIN ORDER
//GND EN DIR 5V

//SENSOR PIN ORDER
//VCC TRIG ECHO GND
int debugMode = 1;
int dirPin = 7;
int enablePin = 8;
int trigPin = 12;
int echoPin = 11;

void setup() {
  pinMode(dirPin, OUTPUT);
  pinMode(enablePin, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(9600);
  _clearReceivedCommand();
}

void loop() {
  byte byteRead;
  while (Serial.available()) {

    byteRead = Serial.read();

    if(byteRead == '\n' || byteRead == '\r'){
      receivedBytes[iterator] = '\0';
      handleReceivedCommand();
      iterator = 0;
    }else{
      receivedBytes[iterator] = byteRead;
      iterator++;
    }

  }
}

void handleReceivedCommand() {
  if (receivedBytes[0] == 0)
    return;

  if(debugMode == 1){
    Serial.print("DEBUG Handling this command: ");
    Serial.println(receivedBytes);
  }

  switch(receivedBytes[0]) {
    case 'n':
      turnOnDeskMove();
      Serial.println("OK");
      break;
    case 'f':
      turnOffDeskMove();
      Serial.println("OK");
      break;
    case 'u':
      //moveDeskUp();
      setDirectionToUp();
      Serial.println("OK");
      break;
    case 'd':
      //moveDeskDown();
      setDirectionToDown();
      Serial.println("OK");
      break;
    case 'p':
      readPosition();
      Serial.println("OK");
      break;
    case 't':
      moveDeskToPos();
      Serial.println("OK");
      break;
    default:
      Serial.print("RECEIVED SOMETHING");
      Serial.println(receivedBytes);
      break;
  };

  _clearReceivedCommand();
}

void _clearReceivedCommand() {
  memset(&receivedBytes[0], 0, CMD_SIZE);
}

void toggleDebugMode() {
  Serial.println("DEBUG Toggling debug mode");
  debugMode = ! debugMode;
  pinMode(13, OUTPUT);
  digitalWrite(13, debugMode);
}

void moveDeskUp() {
  moveDesk(1);
}

void moveDeskDown() {
  moveDesk(0);
}

void setDirectionToUp() {
  turnOffDeskMove();
  digitalWrite(dirPin, LOW);
}

void setDirectionToDown() {
  turnOffDeskMove();
  digitalWrite(dirPin, HIGH);  
}

void turnOnDeskMove() {
  Serial.println("Enable desk move");
  digitalWrite(enablePin, HIGH);
}

void turnOffDeskMove() {
  Serial.println("Disable desk move");
  digitalWrite(enablePin, LOW);
}

void moveDeskToPos() {
  long current_pos = pollSensor();
  const char* target_pos_str = parseDataFromCommandString();
  int target_pos_int = atoi(target_pos_str);
  Serial.print("Target pos: ");
  Serial.println(target_pos_int);
  Serial.print("Current pos before doing anything: ");
  Serial.println(current_pos);

  unsigned long current_time = millis();
  unsigned long last_check = current_time;
  long pos_at_last_check = 0;
  if(target_pos_int - current_pos < 0){
    Serial.println("Desk should be moved up");
    //Move desk up
    setDirectionToUp();
    turnOnDeskMove();
    while(current_pos >= target_pos_int){
      current_pos = pollSensor();
      Serial.print("Updating current_pos to: ");
      Serial.println(current_pos);
      current_time = millis();
      Serial.println(current_time);
      if (current_time - last_check >= 2000) {
        Serial.println("CHECKING FOR MOVEMENT");
        if(abs(current_pos - pos_at_last_check) <= 1){
          turnOffDeskMove();
          Serial.println("Desk not moving, turning off");
          current_pos = target_pos_int - 5;
        }else{
          pos_at_last_check = current_pos;
        }
        last_check = millis();
      }
    }
    Serial.println("Done moving desk up");
  }else if(target_pos_int - current_pos > 0){
    Serial.println("Desk should be moved down");
    //Move desk down
    setDirectionToDown();
    turnOnDeskMove();
    while(current_pos <= target_pos_int){
      current_pos = pollSensor();
      Serial.print("Updating current_pos to: ");
      Serial.println(current_pos);
      current_time = millis();
      if (current_time - last_check >= 2000) {
        Serial.println("CHECKING FOR MOVEMENT");
        if(abs(current_pos - pos_at_last_check) <= 1){
          turnOffDeskMove();
          Serial.println("Desk not moving, turning off");
          current_pos = target_pos_int + 5;
        }else{
          pos_at_last_check = current_pos;
        }
        last_check = millis();
      }
    }
    Serial.println("Done moving desk down");
  }else{
    //Current pos, do nothing
  }
  turnOffDeskMove();
}

void readPosition() {
  long distance = pollSensor();
  Serial.print("Sensor says:");
  Serial.println(distance);
}

long pollSensor() {
  long duration, distance;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;
  return distance;
}

void moveDesk(int direction) {
  //1 = up
  //0 = down
  const char* duration_str = parseDataFromCommandString();
  int duration_int = atoi(duration_str);

  if(direction == 1){
    setDirectionToUp();
  }else{
    setDirectionToDown();
  }

  turnOnDeskMove();
  delay(duration_int);
  turnOffDeskMove();
}

const char* parseDataFromCommandString(){
  const char* data;
  char* delim_char_ptr = strtok(receivedBytes, CMD_DELIMITER);

  if(delim_char_ptr != NULL){
    delim_char_ptr = strtok(NULL, CMD_DELIMITER);
    data = delim_char_ptr;
  }else{
    data = "";
  }

  return data;
}

