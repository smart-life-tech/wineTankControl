#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <EasyNextionLibrary.h>
#include <EEPROM.h> // knihovna pro zapis do pameti eeprom
#include <ESPAsyncWiFiManager.h>
// WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
DNSServer dns;
// Set your Static IP address
IPAddress local_IP(192, 168, 1, 184);
// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);

IPAddress subnet(255, 255, 0, 0);
#define ONE_WIRE_BUS 4 // The one-wire bus pin
#define RELAY_PINS                  \
    {                               \
        0, 5, 18, 25, 19, 3, 18, 18 \
    } // Example2 relay pins

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int numberOfSensors = 8;
const int numRelays = 8;

int relayPins[numRelays] = RELAY_PINS;
float desiredTemperatures[numRelays];   // Array to store desired temperatures for each tank
int currentTankTemperatures[numRelays]; // Array to store current temperatures for each tank
int relayMode[numRelays];

EasyNex myNex(Serial2); // Use the correct Serial port and baud rate
// Adresy cidel
// DeviceAddress tank1, tank2, tank3, tank4, tank5, tank6, tank7, tank8;
DeviceAddress tanks[8];
DeviceAddress tank1 = {0x28, 0xFF, 0xE0, 0x19, 0x2, 0x17, 0x4, 0x95};   // 28 FF E0 19 2 17 4 95
DeviceAddress tank2 = {0x28, 0x58, 0x6, 0x80, 0xE3, 0xE1, 0x3D, 0x5};   // 28 58 6 80 E3 E1 3D 5
DeviceAddress tank3 = {0x28, 0xA, 0xAE, 0x80, 0xE3, 0xE1, 0x3D, 0x48};  // 28 A AE 80 E3 E1 3D 48
DeviceAddress tank4 = {0x28, 0x47, 0xBF, 0x80, 0xE3, 0xE1, 0x3D, 0xC6}; // 28 47 BF 80 E3 E1 3D C6
DeviceAddress tank5 = {0x28, 0xCD, 0xF2, 0x80, 0xE3, 0xE1, 0x3D, 0xE6}; // 28 CD F2 80 E3 E1 3D E6
DeviceAddress tank6 = {0x28, 0xB6, 0xA5, 0x43, 0xD4, 0xE1, 0x3D, 0x57}; // 28 B6 A5 43 D4 E1 3D 57
DeviceAddress tank7 = {0x28, 0x71, 0xBB, 0x80, 0xE3, 0xE1, 0x3D, 0x86}; // 28 71 BB 80 E3 E1 3D 86
DeviceAddress tank8 = {0x28, 0x68, 0x7E, 0x43, 0xD4, 0xE1, 0x3D, 0xAE}; // 28 68 7E 43 D4 E1 3D AE

// promenne teploty
unsigned long casCteniTeplot; // cas posledniho cteni
boolean povolCteniTeplot = false;
byte ID_teploty;                          // ID pro aktualizaci teploty v pripade nastaveni teplot
unsigned long prodlevaCteniTeplot = 5000; // nastaveni periody cteni teploty
byte resolutionDS = 12;                   // nastaveni rozliseni cidla
//------------------------------------------------------------------
// pole adres cidel DS 18B20 - 8 pozic dalsi volna adresa 624
// const int cidlaDS_i[] = {560, 568, 576, 584, 592, 600, 608, 616};
const int cidlaDS_i = 560; // prvni adresa EEPROM pro zapis adres cidel

void printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        // zero pad the address if necessary
        if (deviceAddress[i] < 16)
            Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}

void printAll()
{
    Serial.print("Device 0 Address: ");
    printAddress(tank1);
    Serial.println();
    Serial.print("Device 1 Address: ");
    printAddress(tank2);
    Serial.println();
    Serial.print("Device 2 Address: ");
    printAddress(tank3);
    Serial.println();
    Serial.print("Device 3 Address: ");
    printAddress(tank4);
    Serial.println();
    Serial.print("Device 4 Address: ");
    printAddress(tank5);
    Serial.println();
    Serial.print("Device 5 Address: ");
    printAddress(tank6);
    Serial.println();
    Serial.print("Device 6 Address: ");
    printAddress(tank7);
    Serial.println();
    Serial.print("Device 7 Address: ");
    printAddress(tank8);
    Serial.println();
}

const char *ssid = "TECNO ";
const char *password = "1234567890";

AsyncWebServer server(80);

void copyAddress(DeviceAddress source, DeviceAddress &destination)
{
    for (int i = 0; i < 8; i++)
    {
        destination[i] = source[i];
    }
}

void handleUpdateData(AsyncWebServerRequest *request)
{
    // Assuming you have an array currentTankTemperatures with the temperature data for 8 sensors

    String sensorData[8];

    for (int i = 0; i < 8; i++)
    {
        sensorData[i] = "sensor" + String(i + 1) + ": " + String(desiredTemperatures[i]) + "°C";
    }

    // Build the JSON response
    String response = "{";
    for (int i = 0; i < 8; i++)
    {
        response += "\"" + sensorData[i] + "\"";
        if (i < 7)
        {
            response += ",";
        }
    }
    response += "}";
    String responses = "{\"sensor1\": \"" + String(desiredTemperatures[0]) + "\", \"sensor2\": \"" + String(desiredTemperatures[1]) + "\",  \"sensor3\": \"" + String(desiredTemperatures[2]) + "\", \"sensor4\": \"" + String(desiredTemperatures[3]) + "\", \"sensor5\": \"" + String(desiredTemperatures[4]) + "\", \"sensor6\": \"" + String(desiredTemperatures[5]) + "\", \"sensor7\": \"" + String(desiredTemperatures[6]) + "\", \"sensor8\": \"" + String(desiredTemperatures[7]) + "\"}";

    Serial.println(responses);
    request->send(200, "application/json", responses);
}

void handleUpdateSet(AsyncWebServerRequest *request)
{
    // Assuming you have an array currentTankTemperatures with the temperature data for 8 sensors

    String sensorData[8];

    for (int i = 0; i < 8; i++)
    {
        sensorData[i] = "sensor" + String(i + 1) + ": " + String(currentTankTemperatures[i]) + "°C";
    }
    // Build the JSON response
    String response = "{";
    for (int i = 0; i < 8; i++)
    {
        response += "\"" + sensorData[i] + "\"";
        if (i < 7)
        {
            response += ",";
        }
    }
    response += "}";
    String responses = "{\"sensor1\": \"" + String(currentTankTemperatures[0]) + "\", \"sensor2\": \"" + String(currentTankTemperatures[1]) + "\",  \"sensor3\": \"" + String(currentTankTemperatures[2]) + "\", \"sensor4\": \"" + String(currentTankTemperatures[3]) + "\", \"sensor5\": \"" + String(currentTankTemperatures[4]) + "\", \"sensor6\": \"" + String(currentTankTemperatures[5]) + "\", \"sensor7\": \"" + String(currentTankTemperatures[6]) + "\", \"sensor8\": \"" + String(currentTankTemperatures[7]) + "\"}";

    Serial.println(responses);
    request->send(200, "application/json", responses);
}

void handleGetIPAddress(AsyncWebServerRequest *request)
{
    String ipAddress = WiFi.localIP().toString();
    request->send(200, "text/plain", ipAddress);
}

void handleAdminPage(AsyncWebServerRequest *request)
{
    request->send(SPIFFS, "/admin.html", "text/html");
}

void handleSetAddresses(AsyncWebServerRequest *request)
{
    /*
    String sensor1 = request->arg("sensor1");
    String sensor2 = request->arg("sensor2");
    // Retrieve other sensor addresses as needed

    // Print or use the retrieved sensor addresses
    Serial.print("Sensor 1 Address: ");
    Serial.println(sensor1);
    Serial.print("Sensor 2 Address: ");
    Serial.println(sensor2);
    */
    // Print or use other sensor addresses
    // ...
    // Store tank addresses in the array
    // Copy tank addresses to the array
    copyAddress(tank1, tanks[0]);
    copyAddress(tank2, tanks[1]);
    copyAddress(tank3, tanks[2]);
    copyAddress(tank4, tanks[3]);
    copyAddress(tank5, tanks[4]);
    copyAddress(tank6, tanks[5]);
    copyAddress(tank7, tanks[6]);
    copyAddress(tank8, tanks[7]);

    for (int i = 0; i < 8; i++)
    {
        // Build the argument name for the sensor (e.g., "sensor1", "sensor2", ...)
        String sensorArgName = "sensor" + String(i + 1);

        // Retrieve the sensor address string from the request
        String sensorAddressString = request->arg("sensor" + String(i + 1)); // Replace with your actual sensor data

        // Check if the sensor address string is provided and its length is valid
        if (sensorAddressString.length() > 15)
        { // Check for a valid length (8 bytes in hex with spaces)
            // Convert the sensor address string to a byte array
            byte sensorBytes[8];
            char *ptr;
            for (int j = 0; j < 8; j++)
            {
                sensorBytes[j] = strtol(sensorAddressString.substring(j * 3, j * 3 + 2).c_str(), &ptr, 16);
            }

            // Update the device address for the current sensor
            for (int j = 0; j < 8; j++)
            {
                tanks[i][j] = sensorBytes[j];
            }
        }
    }

    // Print the device addresses for all sensors
    for (int i = 0; i < 8; i++)
    {
        Serial.print("Sensor ");
        Serial.print(i + 1);
        Serial.print(" Device Address: ");
        for (int j = 0; j < 8; j++)
        {
            Serial.print(tanks[i][j], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }
    // request->send(200, "text/plain", "Sensor addresses set successfully!");
    request->send(SPIFFS, "/admin.html", "text/html");
}

// Replaces placeholder with LED state value
String processor(const String &var)
{
    Serial.println(var);

    return String();
}

void setup()
{
    
    Serial.begin(115200);
    Serial.println("Dallas Temperature IC Control Library Demo");
    sensors.begin();
    // locate devices on the bus
    Serial.print("Locating devices...");
    Serial.print("Found ");
    Serial.print(sensors.getDeviceCount(), DEC);
    Serial.println(" devices.");

    // report parasite power requirements
    Serial.print("Parasite power is: ");
    if (sensors.isParasitePowerMode())
        Serial.println("ON");
    else
        Serial.println("OFF");

    // hledaniDS18B20();
    //  method 1: by index
    if (!sensors.getAddress(tank1, 0))
        Serial.println("Unable to find address for Device 0");
    if (!sensors.getAddress(tank2, 1))
        Serial.println("Unable to find address for Device 1");
    if (!sensors.getAddress(tank3, 2))
        Serial.println("Unable to find address for Device 2");
    if (!sensors.getAddress(tank4, 3))
        Serial.println("Unable to find address for Device 3");
    if (!sensors.getAddress(tank5, 4))
        Serial.println("Unable to find address for Device 4");
    if (!sensors.getAddress(tank6, 5))
        Serial.println("Unable to find address for Device 5");
    if (!sensors.getAddress(tank7, 6))
        Serial.println("Unable to find address for Device 6");
    if (!sensors.getAddress(tank8, 7))
        Serial.println("Unable to find address for Device 7");
    // printAll();

    myNex.begin();
    Serial.println("code started !!!");
    delay(1000);
    for (int i = 0; i < numRelays; i++)
    {
        Serial.println(i);
        pinMode(relayPins[i], OUTPUT);
        // Serial.println(i);
        digitalWrite(relayPins[i], LOW);
    }
    for (int i = 0; i < numRelays; i++)
    {
        desiredTemperatures[i] = EEPROM.read(i);
    }
    // Initialize SPIFFS and connect to WiFi
    if (!SPIFFS.begin(true))
    {
        Serial.println("An error occurred while mounting SPIFFS...");
        return;
    }
    // Connect to Wi-Fi
    /* WiFi.begin(ssid, password);
     while (WiFi.status() != WL_CONNECTED)
     {
         delay(1000);
         Serial.println("Connecting to WiFi..");
     }
 */
    bool res;
    AsyncWiFiManager wifiManager(&server, &dns);
    //wifiManager.resetSettings();
    // wifiManager.setAPConfig(IPAddress(10, 0, 1, 1), IPAddress(10, 0, 1, 1), IPAddress(255, 255, 255, 0));
    String macAdd = WiFi.localIP().toString();
    // WIFI_MANAGER_STATION_NAME = macAdd;
    // char  * apNames = macAdd.c_str();
    char apNames[30];
    macAdd.toCharArray(apNames, 30);
    Serial.println(apNames);
    //wifiManager.autoConnect(apNames);
    res = wifiManager.autoConnect(apNames); // password protected ap

    if (!res)
    {
        Serial.println("Failed to connect");
        // ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
    }
    // Configures static IP address
    // if (!WiFi.config(local_IP, gateway, subnet))
    // {
    //    Serial.println("STA Failed to configure");
    // }
    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", String(), false, processor); });

    // Route to load style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/style.css", "text/css"); });
    // Set up server handlers
    server.on("/admin.html", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/admin.html", String(), false, processor); });

    // Serve the User Page
    server.on("/user.html", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/user.html", "text/html"); });

    // Serve the Login Page
    server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/login.html", "text/html"); });

    server.on("/updateData", HTTP_GET, handleUpdateData);
    server.on("/getIpAddress", HTTP_GET, handleGetIPAddress);
    server.on("/setAddresses", HTTP_POST, handleSetAddresses);
    server.on("/updateSet", HTTP_GET, handleUpdateSet);

    // Start the server
    server.begin();
}

void loop()
{
    // server.handleClient();
    //  Add other loop logic here
    // Check for commands from the Nextion display and update desired temperatures
    for (int i = 0; i < numRelays; i++)
    {
        int readTemp = myNex.readNumber("t" + String(i + 1) + "_poz.val");
        if (readTemp < 500)
        {
            desiredTemperatures[i] = readTemp;
            if (readTemp != EEPROM.read(i))
                EEPROM.write(i, desiredTemperatures[i]);
        }
        delay(300);
    }
    for (int i = 0; i < numRelays; i++)
    {
        relayMode[i] = myNex.readNumber("auto" + String(i + 1) + ".val");
        //   Serial.print("relayMode " + String(i)) + " ";
        // Serial.println(relayMode[i]);
        delay(300);
    }
    sensors.requestTemperatures(); // Request temperature readings

    for (int i = 0; i < numRelays; i++)
    {
        int currentTemperature = sensors.getTempCByIndex(i);
        currentTankTemperatures[i] = (currentTemperature);

        // Update the current temperature display on Nextion for each tank
        myNex.writeNum("t" + String(i + 1) + "_akt.val", (currentTemperature));
        /*
                Serial.print("Tank ");
                Serial.print(i + 1);
                Serial.print(" - Current Temperature v5: ");
                Serial.print(currentTemperature);
                Serial.print("°C, Desired Temperature v5: ");
                Serial.print(desiredTemperatures[i]);
                Serial.println("°C");
        */
        if (currentTemperature > desiredTemperatures[i] && relayMode[i] == 10) // 10 = automatic
        {
            // Temperature exceeds desired, turn on the relay for cooling or other actions
            digitalWrite(relayPins[i], HIGH);
        }
        else
        {
            digitalWrite(relayPins[i], LOW);
        }
        delay(300);
        if (relayMode[i] == 20) // manual mode
        {
            digitalWrite(relayPins[i], HIGH);
        }
        else if (relayMode[i] == 30) // nothing set relay off
        {
            digitalWrite(relayPins[i], LOW);
        }
    }
    // EEPROM.commit();/ only for the esp
    delay(1000); // Delay for a second before reading temperatures again
}
