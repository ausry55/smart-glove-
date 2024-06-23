#include <Arduino.h>
#include <Wire.h>

const int flexSensorPins[] = {A1, A2, A3, A4, A5}; // Array of analog pins for the flex sensors
const int numSensors = sizeof(flexSensorPins) / sizeof(flexSensorPins[0]);

int mini = 0;
int midi = 90;
int maxi = 180;

struct Move {
  const char* name;
  int conditions[5][2]; // min and max value for each sensor
};

Move moves[] = {
  {"go", { {maxi, 255}, {maxi, 255}, {0, midi}, {0, midi}, {midi, maxi} }},
  {"want", { {maxi, 255}, {maxi, 255}, {maxi, 255}, {0, midi}, {0, midi} }},
  {"drink", { {midi, maxi}, {midi, maxi}, {midi, maxi}, {0, midi}, {0, midi} }},
  {"happy", { {0, midi}, {midi, maxi}, {midi, maxi}, {midi, maxi}, {0, midi} }},
  {"hungry", { {midi, maxi}, {midi, maxi}, {midi, maxi}, {midi, maxi}, {0, midi} }},
  {"upset", { {0, midi}, {0, midi}, {maxi, 255}, {0, midi}, {midi, maxi} }},
  {"airport", { {0, midi}, {maxi, 255}, {0, midi}, {0, midi}, {midi, maxi} }},
  {"train station", { {maxi, 255}, {maxi, 255}, {maxi, 255}, {midi, maxi}, {maxi, 255} }},
  {"goodbye", { {0, midi}, {0, midi}, {0, midi}, {midi, maxi}, {0, midi} }},
  {"how", { {maxi, 255}, {0, midi}, {0, midi}, {0, midi}, {midi, maxi} }},
  {"where", { {maxi, 255}, {maxi, 255}, {maxi, 255}, {midi, maxi}, {0, midi} }},
  {"what", { {maxi, 255}, {maxi, 255}, {maxi, 255}, {0, midi}, {midi, maxi} }},
  {"hi", { {maxi, 255}, {maxi, 255}, {0, midi}, {0, midi}, {maxi, 255} }},
  {"are you", { {maxi, 255}, {maxi, 255}, {maxi, 255}, {maxi, 255}, {0, midi} }},
  {"that", { {0, midi}, {midi, maxi}, {midi, maxi}, {midi, maxi}, {maxi, 255} }},
  {"to", { {maxi, 255}, {maxi, 255}, {maxi, 255}, {maxi, 255}, {midi, maxi} }},
  {"i am", { {midi, maxi}, {0, midi}, {0, midi}, {0, midi}, {0, midi} }},
};

const int numMoves = sizeof(moves) / sizeof(Move);

void setup() {
  Serial.begin(115200); // Initialize serial communication
//  Serial.println("Arduino is ready.");
}

int mapSensorValue(int sensorValue, int minValue, int maxValue) {
  return map(sensorValue, minValue, maxValue, 0, 250);
}

String readFlexSensors() {
  int flexValues[numSensors];
  for (int i = 0; i < numSensors; i++) {
    flexValues[i] = analogRead(flexSensorPins[i]);
  }
  
  String data = String(flexValues[0]);
  for (int i = 1; i < numSensors; i++) {
    data += "," + String(flexValues[i]);
  }
  
  return data;
}

void printMessage(const char* message) {
  Serial.print(message);
  Serial.println();
}

void loop() {
  // Handle static moves
  int sensorValues[5];
  sensorValues[0] = mapSensorValue(analogRead(flexSensorPins[0]), 785, 890);
  sensorValues[1] = mapSensorValue(analogRead(flexSensorPins[1]), 740, 859);
  sensorValues[2] = mapSensorValue(analogRead(flexSensorPins[2]), 830, 935);
  sensorValues[3] = mapSensorValue(analogRead(flexSensorPins[3]), 745, 840);
  sensorValues[4] = mapSensorValue(analogRead(flexSensorPins[4]), 725, 830);

  for (int i = 0; i < numMoves; i++) {
    bool match = true;
    for (int j = 0; j < 5; j++) {
      if (sensorValues[j] < moves[i].conditions[j][0] || sensorValues[j] > moves[i].conditions[j][1]) {
        match = false;
        break;
      }
    }
    if (match) {
      printMessage(moves[i].name);
      // Send static word to its path
      Serial.print("STATIC_WORD: ");
      Serial.println(moves[i].name);
      break; // Exit loop after finding a match
    }
  }

  delay(2000); // Adjust delay as needed
 
  // Handle new word request from ESP
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove any leading or trailing whitespace
//    Serial.print("Received command: ");
    Serial.println(command);
    
    
    if (command == "READ_SENSORS") {
//      Serial.println("Please perform the move. Reading sensors in 5 seconds...");
      delay(5000);  // Wait for 5 seconds
      String sensorData = readFlexSensors();
      Serial.println(sensorData); // Send sensor data over Serial
    }
  }
}
