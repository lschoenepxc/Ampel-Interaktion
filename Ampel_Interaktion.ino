#include <ESPmDNS.h>

#include <SPIFFS.h>

#include <WiFi.h>
#include <WebServer.h>

#include <Bounce2.h>

#include <heltec.h>

// Display currently out of use: commented out code is usable for OLED display

//#include <Wire.h>               
//#include "HT_SSD1306Wire.h"
//SSD1306Wire  display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED); // addr , freq , i2c group , resolution , rst

Bounce2::Button countBtnBounce = Bounce2::Button();
Bounce2::Button submitBtnBounce = Bounce2::Button();

#define DEMO_DURATION 5000 // sleeping after 5 seconds
typedef void (*Demo)(int);

int demoMode = 0;
int counter[3] = {0,0,0};
// string to save the answers, e.g. 0,0,0 (5 characters plus end-of-string character = 6 characters)
char charCounter[6];
int counterIndex = 0;

// keep track of mean of each answer
float meanAnswers[3];
float nmbAnswers;

// GPIOs for the buttons
int countButton = 1;
int submitButton = 2;

// GPIOs for the LEDs
int greenLed = 3;
int redLed = 4;

// WiFi settings
const char* ssid = "ESP32-Access-Point";
const char* password = "12345678";

WebServer server(80);

IPAddress IP;

void drawFirstQuestion(int nmb) {
  //char str[30];
  //int x = 0;
  //int y = 0;
  //display.setFont(ArialMT_Plain_24);

  // The coordinates define the center of the text
  //display.setTextAlignment(TEXT_ALIGN_CENTER);
  //x = display.width()/2;
  //y = display.height()/2-5;
  //sprintf(str,"1.   %d x", nmb);
  //display.drawString(x, y, str);
}

void drawSecondQuestion(int nmb) {
  //char str[30];
  //int x = 0;
  //int y = 0;
  //display.setFont(ArialMT_Plain_24);

  //display.setTextAlignment(TEXT_ALIGN_CENTER);
  //x = display.width()/2;
  //y = display.height()/2-5;
  //sprintf(str,"2.   %d x", nmb);
  //display.drawString(x, y, str);
}

void drawThirdQuestion(int nmb) {
  //char str[30];
  //int x = 0;
  //int y = 0;
  //display.setFont(ArialMT_Plain_24);

  //display.setTextAlignment(TEXT_ALIGN_CENTER);
  //x = display.width()/2;
  //y = display.height()/2-5;
  //sprintf(str,"3.   %d x", nmb);
  //display.drawString(x, y, str);
}

void drawDanke(int nmb) {
  // Text alignment demo
  //char str[30];
  //int x = 0;
  //int y = 0;
  //display.setFont(ArialMT_Plain_24);

  //display.setTextAlignment(TEXT_ALIGN_CENTER);
  //x = display.width()/2;
  //y = display.height()/2-5;
  //sprintf(str,"Danke!");
  //display.drawString(x, y, str);

  // Replacement for display: currently blinking LEDs
  delay(4000);
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);
  delay(500);
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
  delay(500);
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);
  delay(500);
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
  demoMode = 4;
}

void drawStart(int nmb) {
  //char str[30];
  //int x = 0;
  //int y = 0;
  //display.setFont(ArialMT_Plain_24);

  //display.setTextAlignment(TEXT_ALIGN_CENTER);
  //x = display.width()/2;
  //y = display.height()/2-5;
  //sprintf(str,"Drücke einen Taster um zu starten!");
  //display.drawString(x, y, str);
}

void drawWelcome(int nmb) {
  //char str[30];
  //int x = 0;
  //int y = 0;
  //display.setFont(ArialMT_Plain_24);

  //display.setTextAlignment(TEXT_ALIGN_CENTER);
  //x = display.width()/2;
  //y = display.height()/2-5;
  //sprintf(str,"Es geht los!");
  //display.drawString(x, y, str);

  // Replacement for display: currently blinking LEDs
  digitalWrite(greenLed, HIGH);
  digitalWrite(redLed, HIGH);
  delay(500);
  digitalWrite(greenLed, LOW);
  digitalWrite(redLed, LOW);
  demoMode = 0;
}

void VextON(void)
{
//  needs to be commented in for OLED display
//  pinMode(Vext,OUTPUT);
//  digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
//  needs to be commented in for OLED display
//  pinMode(Vext,OUTPUT);
//  digitalWrite(Vext, HIGH);
}

void saveAnswer(String answer) {
    File file = SPIFFS.open("/answers.txt", FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    file.println(answer);
    file.close();
}

// show the collected data when the esp32 is accessed via http://esp32.local
void handleRoot() {
    File file = SPIFFS.open("/answers.txt");
    if (!file) {
        server.send(500, "text/plain", "Failed to open file for reading");
        return;
    }

    String fileContent;
    while (file.available()) {
        fileContent += String((char)file.read());
    }
    file.close();
    server.send(200, "text/plain", fileContent);
}

// setup bounced button, pinMode and start server
void setup() {

  Serial.begin(115200);
  Serial.println();
  Serial.println();
  VextON();
  delay(100);

  // Initialising the UI will init the display too.
  //display.init();

  //display.setFont(ArialMT_Plain_10);

  countBtnBounce.attach(countButton, INPUT_PULLUP);
  // Entprell Intervall = 5 ms
  countBtnBounce.interval(5);
  countBtnBounce.setPressedState(LOW);

  submitBtnBounce.attach(submitButton, INPUT_PULLUP);
  // Entprell Intervall = 5 ms
  submitBtnBounce.interval(5);
  submitBtnBounce.setPressedState(LOW);

  pinMode(greenLed, OUTPUT);
  pinMode(redLed, OUTPUT);

  // Start file system
  if (!SPIFFS.begin(true)) {
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
  }

  // Start WiFi
  WiFi.softAP(ssid, password);
  IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // make esp available in network via http://esp32.local
  if (!MDNS.begin("esp32")) {
    Serial.println("Error starting mDNS");
    return;
  }

  // Start Web Server
  server.on("/", handleRoot);
  server.begin();
  MDNS.addService("http", "tcp", 80);
}

// functions to display messages on the OLED display
// demoMode keeping track of the current screen via index in this array
Demo demos[] = {drawFirstQuestion, drawSecondQuestion, drawThirdQuestion, drawDanke, drawStart, drawWelcome};

// keeping track of the time since the last button press
long timeSinceLastClick = 0;

void loop() {
  countBtnBounce.update();
  submitBtnBounce.update();

  server.handleClient();

  // Count Button pressed
  if (countBtnBounce.pressed()){
    // if in sleep mode, wake up
    if (demoMode == 3 or demoMode == 4) {
      demoMode = 5;
      timeSinceLastClick = millis();
    }
    // not in sleep mode -> count
    else {
      Serial.println("Taster gedrückt");
      timeSinceLastClick = millis();
      counter[counterIndex]++;
      demoMode = counterIndex;
    }
  }
  
  // Submit Button pressed
  if (submitBtnBounce.pressed()){
    // if in sleep mode, wake up
    if (demoMode == 3 or demoMode == 4) {
      demoMode = 5;
      timeSinceLastClick = millis();
    }
    // not in sleep mode -> submit
    else {
      Serial.println("Submit Taster gedrückt");
      timeSinceLastClick = millis();
      // if not last question, calculate mean and show feedback LED
      if(counterIndex<2){
        meanAnswers[counterIndex]+= counter[counterIndex]/(nmbAnswers+1.0);
        // answer less than mean -> green LED
        if (counter[counterIndex]<=meanAnswers[counterIndex]) {
          digitalWrite(greenLed, HIGH);
          digitalWrite(redLed, HIGH);
          delay(500);
          digitalWrite(greenLed, LOW);
          digitalWrite(redLed, LOW);
          delay(500);
          digitalWrite(greenLed, HIGH);
          digitalWrite(redLed, LOW);
        }
        // answer greater than mean -> red LED
        else {
          digitalWrite(greenLed, HIGH);
          digitalWrite(redLed, HIGH);
          delay(500);
          digitalWrite(greenLed, LOW);
          digitalWrite(redLed, LOW);
          delay(500);
          digitalWrite(redLed, HIGH);
          digitalWrite(greenLed, LOW);
        }
        // next question
        counterIndex++;
        demoMode = counterIndex;
        }
      // last question -> calculate mean, show feedback LED, save answer and reset
      else {
        meanAnswers[counterIndex]+= counter[counterIndex]/(nmbAnswers+1.0);
        if (counter[counterIndex]<=meanAnswers[counterIndex]) {
          digitalWrite(greenLed, HIGH);
          digitalWrite(redLed, HIGH);
          delay(500);
          digitalWrite(greenLed, LOW);
          digitalWrite(redLed, LOW);
          delay(500);
          digitalWrite(greenLed, HIGH);
          digitalWrite(redLed, LOW);
        }
        else {
          digitalWrite(greenLed, HIGH);
          digitalWrite(redLed, HIGH);
          delay(500);
          digitalWrite(greenLed, LOW);
          digitalWrite(redLed, LOW);
          delay(500);
          digitalWrite(redLed, HIGH);
          digitalWrite(greenLed, LOW);
        }
        counterIndex++;
        // Serial.println("Complete Array:");
        sprintf(charCounter,"%d, %d, %d", counter[0],counter[1],counter[2]);
        /// Serial.println(charCounter);
        // create string to save the answer
        sprintf(meanChar,"%.2f, %.2f, %.2f", meanAnswers[0],meanAnswers[1],meanAnswers[2]);
        // Serial.println(meanChar);
        saveAnswer(charCounter);
        nmbAnswers += 1.0;
        // reset
        counterIndex = 0;
        counter[0] = 0;
        counter[1] = 0;
        counter[2] = 0;
        // switch to "Danke" screen
        demoMode = 3;
      }
    }
  }
  // Show submit feedback via blinking LEDs (Replacement for display)
  if (demoMode == 3) 
  {
    demos[demoMode](counter[counterIndex]);
  }
  if (demoMode == 5) 
  {
    demos[demoMode](counter[counterIndex]);
  }
   // commented out code is usable for OLED display
  // clear the display
  //display.clear();

  // draw the current demo method
  //demos[demoMode](counter[counterIndex]);

  //display.setTextAlignment(TEXT_ALIGN_RIGHT);
  //display.drawString(10, 128, String(millis()));

  // write the buffer to the display
  //display.display();

  // if no button is pressed for 5 seconds, go to sleep mode and reset
  if (millis() - timeSinceLastClick > DEMO_DURATION) {
    demoMode = 4;
    counter[0] = 0;
    counter[1] = 0;
    counter[2] = 0;
    counterIndex = 0;
    digitalWrite(greenLed, LOW);
    digitalWrite(redLed, LOW);
  }
  delay(10);
}
