#include "WiFi.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include <FS.h>

AsyncWebServer server(80);
// Function to handle updating sensor data

void handleUpdateData(AsyncWebServerRequest *request)
{
    // Retrieve sensor data from your Arduino
    String sensorData1 = "23°C";
    String sensorData2 = "24°C";
    // ... Retrieve other sensor data as needed

    // Build the JSON response
    String response = "{\"sensor1\": \"" + sensorData1 + "\", \"sensor2\": \"" + sensorData2 + "\"}";
    request->send(200, "application/json", response);
}

void handleGetIPAddress()
{
    String ipAddress = WiFi.localIP().toString();
    server.send(200, "text/plain", ipAddress);
}

void handleAdminPage()
{
    server.send(SPIFFS, "/admin.html", "text/html");
}

void handleSetAddresses()
{
    String sensor1 = server.arg("sensor1");
    String sensor2 = server.arg("sensor2");
    // Retrieve other sensor addresses as needed

    // Print or use the retrieved sensor addresses
    Serial.print("Sensor 1 Address: ");
    Serial.println(sensor1);
    Serial.print("Sensor 2 Address: ");
    Serial.println(sensor2);
    // Print or use other sensor addresses
    // ...

    server.send(200, "text/plain", "Sensor addresses set successfully!");
}

void handleFileRead(String path)
{
    String contentType = "text/plain";
    if (path.endsWith(".html"))
    {
        contentType = "text/html";
    }
    else if (path.endsWith(".css"))
    {
        contentType = "text/css";
    }

    if (SPIFFS.exists(path))
    {
        File file = SPIFFS.open(path, "r");
        server.streamFile(file, contentType);
        file.close();
    }
    else
    {
        server.send(404, "text/plain", "File not found");
    }
}

void handleFileWrite(String path, String content)
{
    File file = SPIFFS.open(path, "w");
    if (!file)
    {
        server.send(500, "text/plain", "Failed to open file for writing");
        return;
    }
    if (file.print(content))
    {
        server.send(200, "text/plain", "File saved successfully");
    }
    else
    {
        server.send(500, "text/plain", "Failed to write to file");
    }
    file.close();
}

void setup()
{
    // Initialize SPIFFS and connect to WiFi
    server.on("/updateData", HTTP_GET, handleUpdateData);
    server.on("/getIpAddress", HTTP_GET, handleGetIPAddress);
    // Set up server handlers
    server.on("/admin", HTTP_GET, handleAdminPage);
    // Set up server handlers to read and write files
    server.on("/html", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/login.html", "text/html"); });
                  // Serve the User Page
    server.on("/user", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/user.html", "text/html"); });
                  // Serve the User Page
    server.on("/user", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/admin.html", "text/html"); });
    server.on("/setAddresses", HTTP_POST, handleSetAddresses);
    // Handle file write request

    server.on("/write", HTTP_POST, []()
              {
    String content = server.arg("content");
    handleFileWrite("/login.html", content); });

    // Start the server
    server.begin();
}

void loop()
{
    server.handleClient();
    // Add other loop logic here
}
