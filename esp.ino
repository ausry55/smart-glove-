#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <ESP8266HTTPClient.h>

#define FIREBASE_HOST "https://the-glove-test-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "sCgqa5q2XUa7CtuCXqOM3Qr0rcNOBAqFIe2EHCtZ"

#define WIFI_SSID "lol"
#define WIFI_PASSWORD "mad max 2015"

int values[5];
String sensorData = "";

// Declare the Firebase Data object in the global scope
FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);
  Serial.println("Serial communication started\n\n");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP()); //print local IP address

  /* Assign the database URL and database secret(required) */
  config.database_url = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);
  delay(1000);
}

void loop() {
  // Check the value of "check" in Firebase
  if (Firebase.ready()) {
    if (Firebase.getInt(firebaseData, "/check")) {
      int checkValue = firebaseData.intData();

      if (checkValue == 1) {
        Serial.println("New Word Available . Proceeding with operations.");

        // Get the new word from Firebase
        if (Firebase.getString(firebaseData, "/new")) {
          Serial.print("Value of 'new' from Firebase: ");
          String wordd = firebaseData.stringData();
          Serial.println(wordd);

          if (wordd != "") {
            // Send command to Arduino to read sensor values
            Serial.println("READ_SENSORS");
            delay(1000);  // Wait for Arduino to respond
            // Wait until there is data available on Serial
        if (Serial.available()) {
              sensorData = Serial.readStringUntil('\n');
              Serial.println(sensorData);
           }
            // Read data from Serial
            sensorData = Serial.readStringUntil('\n');
              Serial.println(sensorData);

            if (sensorData.length() > 0) {
              splitString(sensorData);
              sensorData = "";

              // Save the sensor data to Firebase at the specific path
              String path = "/readings/signal-" + wordd;
              for (int i = 0; i < 5; i++) {
                if (Firebase.setInt(firebaseData, path + "/sensor" + (i + 1), values[i])) {
                  Serial.println("Data of sensor " + String(i + 1) + " saved successfully");
                } else {
                  Serial.println("Failed to save data of sensor " + String(i + 1));
                  Serial.println(firebaseData.errorReason());
                }
              }

              // Clear the 'new' value in Firebase
              if (Firebase.setString(firebaseData, "/new", "")) {
                Serial.println("Cleared 'new' value in Firebase");
              } else {
                Serial.println("Failed to clear 'new' value in Firebase");
                Serial.println(firebaseData.errorReason());
              }
            }
          } else {
            Serial.println("Error getting 'new' value from Firebase");
            Serial.println(firebaseData.errorReason());
          }

          // Reset the 'check' value to 0
          if (Firebase.setInt(firebaseData, "/check", 0)) {
            Serial.println("Reset 'check' value to 0 in Firebase");
          } else {
            Serial.println("Failed to reset 'check' value in Firebase");
            Serial.println(firebaseData.errorReason());
          }
        }
      } else {
        // Handle static words from Arduino
        if (Serial.available()) {
          String staticWord = Serial.readStringUntil('\n');
          staticWord.trim(); // Remove any leading or trailing whitespace
          if (staticWord.startsWith("STATIC_WORD: ")) {
            String movee = staticWord.substring(13); // Extract the word after "STATIC_WORD: "
//            Serial.println(movee);
            Firebase.setString(firebaseData, "/static", movee);
          }
        }
      }
    } else {
      Serial.println("Error getting 'check' value from Firebase");
      Serial.println(firebaseData.errorReason());
    }
  } else {
    Serial.println("Firebase not ready");
  }

  // Delay to prevent rapid polling
  delay(1000);
}

void splitString(String input) {
  int index = 0;
  int start = 0;
  int end = input.indexOf(',');

  while (end != -1) {
    values[index] = input.substring(start, end).toInt();
    index++;
    start = end + 1;
    end = input.indexOf(',', start);
  }
  values[index] = input.substring(start).toInt(); // Get the last value
}
