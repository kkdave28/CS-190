#include <Arduino.h>
#include <Adafruit_CCS811.h>
#include <Adafruit_VC0706.h>
#include <SoftwareSerial.h>
#include <FS.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#define TRIG 13
#define ECHO 12
#define DBG_OUTPUT_PORT Serial
/*
    Implemented the HCSRO4 Sensor
    PIN CONFIGS:
        PIN D-13 - TRIGGER
        PIN D-12 - ECHO
    Functions Used:
       void setup_UltrasonicSensor() --> initiates the pins
       unsigned long get_distance() --> returns an unsigned long which when multiplied by 0.017, gives the distance.
       void print_USData() --> prints the distance in a human readable format to the terminal.
    
    Implementing the CCS811 Sensor
    PIN CONFIGS:
        SDA [PIN D2] --> SDA On the Sensor
        SCL/SCLK[PIN D14] --> SCL On the Sensor
        USE ONLY 3.3V to power the Sensor
        Ground to Ground
        Ground the WAKE PINOUT on the Sensor.
    Functions Used:
        void setup_airqsensor() --> Sets up the air quality sensor
        void print AQSData() --> Prints the CO2 content and VOC level in a human readable format to the terminal
*/
unsigned long duration;
int distance;
Adafruit_CCS811 AirQualitySensor;
SoftwareSerial camera_connection = SoftwareSerial(4,5);
Adafruit_VC0706 main_cam = Adafruit_VC0706(&camera_connection);
ESP8266WebServer server(80);
File fsUploadFile; 
//int num_snapshots = 3;
const char * ssid = "UCInet Mobile Access";
const char * password = "";
const char * host = "esp8266fs";

// SERVER CODE

String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.htm";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!SPIFFS.exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (SPIFFS.exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = SPIFFS.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }

  String path = server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") {
      output += ',';
    }
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}
// SERVER CODE END
void setup_flash()
{
    SPIFFS.begin();
}
void setup_camera()
{
    if(main_cam.begin())
    {
        Serial.println("Camera Found");
    }
    else 
    {
        Serial.println("Camera Not found...");
        return;
    }
    char * version = main_cam.getVersion();
    if(version)
    {
        Serial.print("Camera Version:  ");
        Serial.println(version);
    }
    else
    {
        Serial.println("Failed to get version");
    }
    main_cam.setImageSize(VC0706_160x120);
    uint8_t img_sz = main_cam.getImageSize();
    switch (img_sz)
    {
        case VC0706_160x120:
            Serial.println("Image size = 160x120");
            break;
        case VC0706_320x240:
            Serial.println("Image size = 320x240");
            break;
        case VC0706_640x480:
            Serial.println("Image size = 640x480");
            break;
        default:
            Serial.println("Failed to get image size");
    }

}
void setup_airqsensor()
{
    if(!AirQualitySensor.begin())
    {
        Serial.println("Not Detected Air Quality Sensor");
        while(1);
    }
}

void setup_UltrasonicSensor()
{
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);    
    distance = 0;
    duration =0;
}
unsigned long get_distance()
{
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);

    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);

    duration = pulseIn(ECHO, HIGH);
    return duration;
}
void take_snapshot()
{
    File image_file = SPIFFS.open("/img-1.jpg", "w");
    if(!main_cam.takePicture())
    {
        Serial.println("Failed to take a snapshot");
        return;
    }
    else
    {
        Serial.println("Picture successfully taken!");
    }
    int jpeglen = main_cam.frameLength();
    Serial.print("Storing ");
    Serial.println(jpeglen, DEC);
    Serial.println(" byte image...");
    byte write_count = 0;
    while(jpeglen > 0)
    {
        uint8_t * buf;
        uint8_t bytes_to_read = min(32, jpeglen);
        buf = main_cam.readPicture(bytes_to_read);
        image_file.write(buf, bytes_to_read);
        if(++write_count >= 64)
        {
            Serial.print(".");
            write_count = 0;
        }
        jpeglen-=bytes_to_read;
    }
    image_file.close();
}
void setup_server()
{
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }


  //WIFI INIT
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
  }
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());

  MDNS.begin(host);
  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");


  //SERVER INIT
  //list directory
  server.on("/list", HTTP_GET, handleFileList);
  //load editor
  server.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });
  //create file
  server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  server.on("/edit", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");
}
void setup() 
{
    //setup_UltrasonicSensor();
    //Serial.begin(9600);

    //setup_airqsensor();
    setup_flash();
    setup_camera();
    //while(num_snapshots--)
    take_snapshot();
//    Serial.end();
    setup_server();
    // put your setup code here, to run once:
}
void print_USData()
{
    distance = get_distance()*0.034/2;
    Serial.print("Distance: ");
    Serial.println(distance);
}
void print_AQSData()
{
    if(AirQualitySensor.available())
    {
        if(!AirQualitySensor.readData())
        {
            Serial.print("CO2 = ");
            Serial.println(AirQualitySensor.geteCO2());
            Serial.print("VOC = ");
            Serial.println(AirQualitySensor.getTVOC());
        }
        else
        {
            Serial.println("ERROR READING AIR QUALITY SENSOR!!!");
            while(1);
        }
    }
}

void loop() {
    
    //print_USData();
    //print_AQSData();
    server.handleClient();
    //delay(1500);



    // put your main code here, to run repeatedly:
}