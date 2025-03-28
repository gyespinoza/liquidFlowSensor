#include "Credentials.h"
#define ESP32_MYSQL_DEBUG_PORT Serial
// Debug Level from 0 to 4
#define _ESP32_MYSQL_LOGLEVEL_ 1
#include <ESP32_MySQL.h>
#define USING_HOST_NAME true
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60       /* Time ESP32 will go to sleep (in seconds) (1 minute)*/
#include <Arduino.h>
#include <SensirionI2cSf06Lf.h>
#include <Wire.h>

SensirionI2cSf06Lf sensor;

static char errorMessage[64];
static int16_t error;

void print_byte_array(uint8_t* array, uint16_t len) {
  uint16_t i = 0;
  Serial.print("0x");
  for (; i < len; i++) {
    Serial.print(array[i], HEX);
  }
}

char server[] = "192.168.137.1";  //server IP (change with server hostname/URL)
uint16_t server_port = 3306;      //MySQL port
char database[] = "lc";           //DB name
char table[] = "sensor_flujo";    //table name

// variables para insercion de datos
String identificador;
String INSERT_SQL;
String aFlowLecture;
String aTemperatureLecture;


ESP32_MySQL_Connection conn((Client*)&client);
ESP32_MySQL_Query* query_mem;


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }
  Wire.begin();
  sensor.begin(Wire, SLF3C_1300F_I2C_ADDR_08);

  sensor.stopContinuousMeasurement();
  delay(100);
  uint32_t productIdentifier = 0;
  uint8_t serialNumber[8] = { 0 };
  error = sensor.readProductIdentifier(productIdentifier, serialNumber, 8);
  if (error != NO_ERROR) {
    Serial.print("Error trying to execute readProductIdentifier(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
  }
  Serial.print("productIdentifier: ");
  Serial.print(productIdentifier);
  Serial.print("\t");
  Serial.print("serialNumber: ");
  print_byte_array(serialNumber, 8);
  Serial.println();


  error = sensor.startH2oContinuousMeasurement();
  if (error != NO_ERROR) {
    Serial.print(
      "Error trying to execute startH2oContinuousMeasurement(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
  }

  //test connection MySQL
  while (!Serial && millis() < 5000)
    ;  // wait for serial port to connect
  ESP32_MYSQL_DISPLAY1("\nStarting Basic_Insert_ESP on", ARDUINO_BOARD);
  // Begin WiFi section
  ESP32_MYSQL_DISPLAY1("Connecting to", ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    ESP32_MYSQL_DISPLAY0(".");
  }

  // print out info about the connection:
  ESP32_MYSQL_DISPLAY1("Connected to network. My IP address is:", WiFi.localIP());
  ESP32_MYSQL_DISPLAY3("Connecting to SQL Server @", server, ", Port =", server_port);
  ESP32_MYSQL_DISPLAY5("User =", user, ", PW =", password, ", DB =", database);

  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");
}

void runInsert() {
  // Initiate the query class instance
  ESP32_MySQL_Query query_mem = ESP32_MySQL_Query(&conn);

  if (conn.connected()) {
    ESP32_MYSQL_DISPLAY(INSERT_SQL);

    // Execute the query
    if (!query_mem.execute(INSERT_SQL.c_str())) {
      ESP32_MYSQL_DISPLAY("Insert error");
    } else {
      ESP32_MYSQL_DISPLAY("Data Inserted.");
    }
  } else {
    ESP32_MYSQL_DISPLAY("Disconnected from Server. Can't insert.");
  }
}

void loop() {
  float aFlow = 0.0;
  float aTemperature = 0.0;
  uint16_t aSignalingFlags = 0u;
  delay(20);
  error = sensor.readMeasurementData(INV_FLOW_SCALE_FACTORS_SLF3C_1300F,
                                     aFlow, aTemperature, aSignalingFlags);


  if (error != NO_ERROR) {
    Serial.print("Error trying to execute readMeasurementData(): ");
    errorToString(error, errorMessage, sizeof errorMessage);
    Serial.println(errorMessage);
    return;
  }

  //asignar valores
  aFlowLecture = String(aFlow);
  aTemperatureLecture = String(aTemperature);

  /*Serial.print("aFlow: ");
  Serial.print(aFlow);
  Serial.print("\t");
  Serial.print("aTemperature: ");
  Serial.print(aTemperature);
  Serial.print("\t");
  Serial.print("aSignalingFlags: ");
  Serial.print(aSignalingFlags);
  Serial.println();*/

  identificador = "SLF3S-4000B";
  INSERT_SQL = String("INSERT INTO ") + database + "." + table
               + "( identificador, temperatura, flujo) VALUES ('" + identificador + "', '" + aTemperatureLecture + "', '" + aFlowLecture + "')";  //columnas= identificador y variable*/

  ESP32_MYSQL_DISPLAY("Connecting...");
  //if (conn.connect(server, server_port, user, password))
  if (conn.connectNonBlocking(server, server_port, user, password) != RESULT_FAIL) {
    delay(500);
    runInsert();
    conn.close();  // close the connection
  } else {
    ESP32_MYSQL_DISPLAY("\nConnect failed. Trying again on next iteration.");
  }

  ESP32_MYSQL_DISPLAY("\nSleeping...");
  ESP32_MYSQL_DISPLAY("================================================");


  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  Serial.flush();
  esp_deep_sleep_start();
}


/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0: Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1: Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER: Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD: Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP: Serial.println("Wakeup caused by ULP program"); break;
    default: Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
}
