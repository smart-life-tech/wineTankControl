#include <OneWire.h>
#include <DallasTemperature.h>
#include <EasyNextionLibrary.h>

#define ONE_WIRE_BUS 2 // The one-wire bus pin
#define RELAY_PINS              \
    {                           \
        23, 1, 3, 19, 25, 18, 5, 0 \
    } // Example2 relay pins

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int numRelays = 8;
int relayPins[numRelays] = RELAY_PINS;
float desiredTemperatures[numRelays];     // Array to store desired temperatures for each tank
float currentTankTemperatures[numRelays]; // Array to store current temperatures for each tank

EasyNex myNex(Serial2); // Use the correct Serial port and baud rate

void setup()
{
    Serial.begin(9600);
    sensors.begin();

    for (int i = 0; i < numRelays; i++)
    {
        pinMode(relayPins[i], OUTPUT);
        digitalWrite(relayPins[i], LOW);
    }

    myNex.begin();
}

void loop()
{
    // Check for commands from the Nextion display and update desired temperatures
    for (int i = 0; i < numRelays; i++)
    {
        desiredTemperatures[i] = myNex.readNumber("tank" + String(i + 1) + ".val");
    }

    sensors.requestTemperatures(); // Request temperature readings

    for (int i = 0; i < numRelays; i++)
    {
        float currentTemperature = sensors.getTempCByIndex(i);
        currentTankTemperatures[i] = currentTemperature;

        // Update the current temperature display on Nextion for each tank
        myNex.writeNum("tank" + String(i + 1) + "temp.val", currentTemperature);

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
    }

    delay(1000); // Delay for a second before reading temperatures again
}
