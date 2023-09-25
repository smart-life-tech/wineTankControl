#include <OneWire.h>
#include <DallasTemperature.h>
#include <EasyNextionLibrary.h>
#include <EEPROM.h>    // knihovna pro zapis do pameti eeprom
#define ONE_WIRE_BUS 4 // The one-wire bus pin
#define RELAY_PINS                 \
  {                              \
    23, 1, 3, 19, 25, 18, 5, 0 \
  } // Example2 relay pins

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int numberOfSensors = 8;
const int numRelays = 8;
int relayPins[numRelays] = RELAY_PINS;
float desiredTemperatures[numRelays];     // Array to store desired temperatures for each tank
float currentTankTemperatures[numRelays]; // Array to store current temperatures for each tank

EasyNex myNex(Serial2); // Use the correct Serial port and baud rate
// Adresy cidel
DeviceAddress tank1, tank2, tank3, tank4, tank5, tank6, tank7, tank8;
/*
  DeviceAddress tank1 = {0x28, 0xFF, 0xE0, 0x19, 0x2, 0x17, 0x4, 0x95}; // 28 FF E0 19 2 17 4 95
  DeviceAddress tank2 = {0x28, 0xFF, 0x2A, 0xA6, 0x1, 0x17, 0x4, 0xE};  // 28 FF 2A A6 1 17 4 E
  DeviceAddress tank3 = {0x28, 0x1F, 0xBD, 0xCB, 0x04, 0x00, 0x00, 0xF1};
  DeviceAddress tank4 = {0x28, 0x60, 0x67, 0xCE, 0x04, 0x00, 0x00, 0x2F};
  DeviceAddress tank5 = {0x28, 0x1F, 0xBD, 0xCB, 0x04, 0x00, 0x00, 0xF1};
  DeviceAddress tank6 = {0x28, 0x60, 0x67, 0xCE, 0x04, 0x00, 0x00, 0x2F};
  DeviceAddress tank7 = {0x28, 0x1F, 0xBD, 0xCB, 0x04, 0x00, 0x00, 0xF1};
  DeviceAddress tank8 = {0x28, 0x60, 0x67, 0xCE, 0x04, 0x00, 0x00, 0x2F};
*/
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
void hledaniDS18B20()
{
  byte address[8];
  byte id;
  oneWire.reset_search(); // resetuj hledani
  if (oneWire.search(address))
  { // hledej adresy
    for (id = 0; id < numberOfSensors; id++)
    { // zapis data do displeje
      String stringText;
      for (byte i = 0; i < 8; i++)
      { // precti a uloz adresy
        EEPROM.write(cidlaDS_i + (id * 8) + i, address[i]);
        stringText += address[i], DEC;
        if (i < 8 - 1)
          stringText += ",";
      }
      Serial.println("Cidlo " + (String)(id + 1) + ": " + stringText);
      if (!oneWire.search(address))
      {
        oneWire.reset_search();
        Serial.println(F("Zadne dalsi cidlo nenalezeno"));
        break;
      }
    }
    Serial.println("Nalezeno " + (String)(id + 1) + " cidel.");
    Serial.println();
  }
}
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
void printAll() {
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
void setup()
{
  Serial.begin(9600);
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

  oneWire.reset_search();
  //assigns the first address found to tank1
  if (!oneWire.search(tank1)) Serial.println("Unable to find address for tank1");
  //assigns the seconds address found to tank2
  if (!oneWire.search(tank2)) Serial.println("Unable to find address for tank2");
  //assigns the first address found to tank1
  if (!oneWire.search(tank3)) Serial.println("Unable to find address for tank3");
  //assigns the seconds address found to tank2
  if (!oneWire.search(tank4)) Serial.println("Unable to find address for tank4");
  //assigns the first address found to tank1
  if (!oneWire.search(tank5)) Serial.println("Unable to find address for tank5");
  //assigns the seconds address found to tank2
  if (!oneWire.search(tank6)) Serial.println("Unable to find address for tank6");
  //assigns the first address found to tank1
  if (!oneWire.search(tank7)) Serial.println("Unable to find address for tank7");
  //assigns the seconds address found to tank2
  if (!oneWire.search(tank8)) Serial.println("Unable to find address for tank8");
  printAll();
  //hledaniDS18B20();
  // method 1: by index
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

  for (int i = 0; i < numRelays; i++)
  {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], LOW);
  }

  myNex.begin();
  Serial.println("code started !!!");
  delay(1000);
}

void loop()
{
  // Check for commands from the Nextion display and update desired temperatures
  for (int i = 0; i < numRelays; i++)
  {
    desiredTemperatures[i] = myNex.readNumber("t" + String(i + 1) + "1_poz.val");
    delay(300);
  }

  sensors.requestTemperatures(); // Request temperature readings

  for (int i = 0; i < numRelays; i++)
  {
    float currentTemperature = sensors.getTempCByIndex(i);
    currentTankTemperatures[i] = currentTemperature;

    // Update the current temperature display on Nextion for each tank
    myNex.writeNum("t" + String(i + 1) + "_akt.val", currentTemperature);

    Serial.print("Tank ");
    Serial.print(i + 1);
    Serial.print(" - Current Temperature: ");
    Serial.print(currentTemperature);
    Serial.print("°C, Desired Temperature: ");
    Serial.print(desiredTemperatures[i]);
    Serial.println("°C");

    if (currentTemperature > desiredTemperatures[i])
    {
      // Temperature exceeds desired, turn on the relay for cooling or other actions
      digitalWrite(relayPins[i], HIGH);
    }
    else
    {
      digitalWrite(relayPins[i], LOW);
    }
    delay(300);
  }

  delay(1000); // Delay for a second before reading temperatures again
}
