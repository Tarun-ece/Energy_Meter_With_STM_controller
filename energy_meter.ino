include<ACON_STM32>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <PZEM004Tv30.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <ThingSpeak.h>
#include <SPI.h>

WiFiClient  client;
PZEM004Tv30 pzem(D7, D8); //RX/TX
LiquidCrystal_I2C lcd(0x27, 16, 2);
ESP8266WebServer server(80);

float voltage, current, power, energy, frequency, pf, units;

unsigned long myChannelNumber = 1519974; //your channel number here
const char * myWriteAPIKey = "9OYRK9BAB6SV9HG1"; //API Key for cloud communication

float number1 = 0.0000f;
float number2 = 0.0000f;
float number3 = 0.0000f;
float number4 = 0.0000f;
float number5 = 0.0000f;
float number6 = 0.0000f;
float number7 = 0.000000f;
thinkspeak=temp;

String myStatus = "";

/*Put your SSID & Password*/
const char* ssid = "*****";  // Enter SSID here
const char* password = "*****";  //Enter Password here

uint8_t LED1pin = D6;         //GPIO 12 OR D6
bool LED1status = LOW;

float consum = 0.000000;
int timer = 0;
int testPin = 0;
int ExternalPin = D5;          //GPIO 14 OR D5
int eeAddress = 10;
uint32_t tsLastReport = 0;

volatile unsigned long timer0_millis;
String page = "";
String Units = "";
int dis = 0;
float eepromdata=0.00000f;
int loopcheck=0;
//int thingspeak =0;
float floatingpoint_variable = 0.000000;
int EEaddress = 0;


ICACHE_RAM_ATTR void detectsMovement() {
    Serial.println("EEPROM Data Writing.... ");
    floatingpoint_variable= eepromdata;

    EEPROM.put(EEaddress,floatingpoint_variable); // Writes the value 3.141592654 to EEPROM
    EEPROM.commit();

    Serial.print("\t eepromdata EEPROM data: ");
    Serial.print(float(floatingpoint_variable),6);
}

void EEPROMloop() {
    Serial.println("EEPROM Data Writing.... ");
    floatingpoint_variable= eepromdata;

    EEPROM.put(EEaddress,floatingpoint_variable); // Writes the value 3.141592654 to EEPROM
    EEPROM.commit();

    Serial.print("\t eepromdata EEPROM data: ");
    Serial.print(float(floatingpoint_variable),6);
}


void setup() {
    Serial.begin(115200);
    EEPROM.begin(32);

    attachInterrupt(digitalPinToInterrupt(ExternalPin), detectsMovement, FALLING);
    delay(3000);
    Serial.print("Connecting to ");
    Serial.print(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected..!");
    Serial.print("Got IP: ");
    Serial.println(WiFi.localIP());

    Serial.print(" ");
    Serial.print(" ");
    Serial.print("EEPROM contents at Address=8 is  : ");
    floatingpoint_variable = 0; // To prove it read from EEPROM!
    EEPROM.get(EEaddress,floatingpoint_variable);
    Serial.println(floatingpoint_variable,8);

    float readParam = floatingpoint_variable;
    Serial.print("\t Consum EEPROM: ");
    Serial.print(float(readParam),6);
    consum = readParam;

      lcd.init();                      // initialize the lcd 
  lcd.init();
  lcd.backlight();

  lcd.setCursor(5, 0);
  lcd.print("ENERGY METER");

    timer = millis();

    testPin = digitalRead(ExternalPin);
    delay(200);
    server.on("/", handle_OnConnect);
    server.on("/led1on", handle_led1on);
    server.on("/led1off", handle_led1off);
    server.onNotFound(handle_NotFound);

    server.begin();
    ThingSpeak.begin(client);
    Serial.println("HTTP server started");
    consum = 80;
}
void loop() {
    timer = millis() - tsLastReport;
    int timercheck = millis();
    float sec = timer;

//    voltage = pzem.voltage();
//    current = pzem.current();
//    frequency = pzem.frequency();
//    pf = pzem.pf();

    voltage = random(224,230);
    current = random(1.5,2.3);  
    frequency = 50;


    float Seconds = 0.00;
    float Minutes = 0.00;
    float Hours = 0.00;
    float Energy = 0.0000;

    Seconds = ((float)sec / 1000);

    tsLastReport = millis();

    Minutes = (float)Seconds / 60;
    Hours = (float)Minutes / 60;
    String b = String(voltage);


//    if(a=="nan" && b=="nan") {
//        if(loopcheck==0) {
//            EEPROMloop();
//            loopcheck=1;
//        }
//    }
    if(a == "nan") {
        Energy = 0.000;
    }

    else {
        consum = float(consum) + float(Energy);
        if(consum !=0) {
            eepromdata=float(consum);
            loopcheck=0;
        }
        else {
            detectsMovement();
        }
    }

    Serial.print("\t Consum Test: ");
    Serial.print(float(consum),5);
    Serial.print("c");

    Units = String(consum,5);

    Serial.print("\t Voltage: ");
    Serial.print(voltage);
    Serial.print("V");
    Serial.print("\t Current: ");
    Serial.print(current);
    Serial.print("A");
    Serial.print("\tFrequency: ");
    Serial.print(frequency, 1);
    Serial.print("Hz");
    Serial.print("\t Seconds : ");
    Serial.print(Seconds, 5);
    Serial.print("\t Minutes : ");
    Serial.print(Minutes, 5);
    Serial.print("\t Hours : ");
    Serial.println(Hours, 5);

    server.handleClient();
   
    lcd.clear();

    number1 = voltage;
    lcd.setCursor(0, 0);
    lcd.print("V=");
    lcd.print(voltage, 2);

    number2 = current;
    lcd.setCursor(9, 0);
    lcd.print("A=");
    lcd.print(current, 4);
    number3= frequency;
    
    if(temp==30) {
        number1 = voltage;
        number2 = current;
        number3 = frequency;


        temp.setField(1, number1);
        temp.setField(2, number2);
        temp.setField(3, number3);
        temp.setStatus(myStatus);

        int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

        if (x == 200) {
            Serial.println("Channel update successful.");
        }
        else {
            Serial.println("Problem updating channel. HTTP error code " + String(x));
        }
        temp=0;
    }

    if (LED1status==HIGH)
    {
        digitalWrite(LED1pin, HIGH);
        delay(2000);
        digitalWrite(LED1pin, LOW);
    }
    else
    {
        digitalWrite(LED1pin, LOW);
    }

    testPin = digitalRead(ExternalPin);
    dis++;
    temp++;
    Serial.println("*********************************");
    Serial.println();
    delay(800);
}


void handle_OnConnect() {
    LED1status = LOW;
    server.send(200, "text/html", SendHTML(voltage, current, power, frequency, pf, Units, LED1status));
}

void handle_led1on() {
    LED1status = HIGH;
    Serial.println("GPIO12 Status: ON");
    server.send(200, "text/html", SendHTML(voltage, current, power, frequency, pf, Units, true));
}

void handle_led1off() {
    LED1status = LOW;
    Serial.println("GPIO12 Status: OFF");
    server.send(200, "*****", SendHTML(voltage, current,frequency, false)); // your application name
}

void handle_NotFound() {
    server.send(404, "text/plain", "Not found");
}
