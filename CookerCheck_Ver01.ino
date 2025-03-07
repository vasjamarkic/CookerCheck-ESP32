#include <WiFi.h>
// https://lastminuteengineers.com/esp32-deep-sleep-wakeup-sources/
// !!!While uploading the code to ESP32, must press the BOOT button!!!
#include "DHT.h"                                                        /*install the DHT library of Adafruit! */
#define DHTPIN 18                                                       /* must not be pin 2 -->  Tx & Rx reserved! */
#define DHTTYPE DHT11
#define Threshold 30                                                    /* Greater the value, more the sensitivity for deep sleep wake up*/
#define LED_ESP 2                                                       /* Signalisation of IR sensor working - board BLUE led */
const int RED = 13, GREEN = 12, BLUE = 14, BUZZ = 4;                    /* RGB LEDs for status information*/
bool stevec_start = 0;
float t_REF=1000.0, t;                                   /*t_REF --> REFERENCE temperature measured at start-up, t --> periodically taken temperature */
float dT = 1.5;                                   /*prag zaznavanje kuhanja (med 1 in 3°C) */
int fire_on_counter = 0, fire_off_counter = 0;
int IR = 5;                                       /*digital input za PIR senzor (onemogoči buzzer/alarm, ko smo blizu in kuhamo). */
// VPIS POMEMBNIH PARAMETROV:
int hot_min = 2;                                  /* perioda v min za alarmiranje */
int cold_min = 5;                                 /* perioda v min za samodejni izklop ob hladnem stanju */

DHT dht(DHTPIN, DHTTYPE);

RTC_DATA_ATTR int bootCount = 0;
touch_pad_t touchPin;

const char* ssid = "Xiaomi 11 Lite 5G NE";
const char* password ="vavava11";

/*----------method---for---print---wakeup-----reason-------------------------*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}
/*--------method--for--printing---whitch---GPIO---was---touched--------------*/
void print_wakeup_touchpad(){
  touchPin = esp_sleep_get_touchpad_wakeup_status();

  switch(touchPin)
  {
    case 0  : Serial.println("Touch detected on GPIO 4"); break;
    case 1  : Serial.println("Touch detected on GPIO 0"); break;
    case 2  : Serial.println("Touch detected on GPIO 2"); break;
    case 3  : Serial.println("Touch detected on GPIO 15"); break;
    case 4  : Serial.println("Touch detected on GPIO 13"); break;
    case 5  : Serial.println("Touch detected on GPIO 12"); break;
    case 6  : Serial.println("Touch detected on GPIO 14"); break;
    case 7  : Serial.println("Touch detected on GPIO 27"); break;
    case 8  : Serial.println("Touch detected on GPIO 33"); break;
    case 9  : Serial.println("Touch detected on GPIO 32"); break;
    default : Serial.println("Wakeup not by touchpad"); break;
  }
}
/*----------------------------------------------------------------------------*/
void callback(){
  //placeholder callback function
}
/*------method---for-----toggle-------LED-----and----BUZZ---------------------*/
void toggleLED(int pin) {                  //funkcija za utripanje LED (toggle)
  digitalWrite(pin, !digitalRead(pin));
}
/*----------------------------------------------------------------------------*/
void setup() {
  // put your setup code here, to run once:
    Serial.begin(115200);
    dht.begin();
    pinMode (RED, OUTPUT);
    pinMode (GREEN, OUTPUT);
    pinMode (BLUE, OUTPUT);
    pinMode (BUZZ, OUTPUT);
    pinMode (IR, INPUT);
    pinMode(LED_ESP,OUTPUT);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("\nConnecting");
    //Increment boot number and print it every reboot
    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));
    //Print the wakeup reason for ESP32 and touchpad too
    print_wakeup_reason();
    print_wakeup_touchpad();
    //Setup interrupt on Touch Pad 3 (GPIO15)
    touchAttachInterrupt(T3, callback, Threshold);
    //Configure Touchpad as wakeup source
    esp_sleep_enable_touchpad_wakeup();
    }
    
void loop() {
  //Izvedba referenčne meritve - samo 1x ob zagonu ali v primeru neuspelega zajema temperature ali v primeru tipke EN
  if (stevec_start == 0 or t_REF == 1.0/0.0 or bootCount == 1) {
    toggleLED(BLUE);
    delay(2000);
    toggleLED(BLUE);
    float t = dht.readTemperature();   
    t_REF = t;   //prenos ref. temp. v t_REF
    digitalWrite(GREEN, HIGH);
    for(int k=0;k<9;k++) {
    //izmenično utripanje  zelene led 5x
    toggleLED(GREEN);
    delay(250);
    bootCount = 0;    //reset števca za ponovno možnost vstopa v to vejitev za zajem referenčne temperature
    }
    Serial.println("Zajem referenčne meritve.");
    stevec_start = 1;   //blokiranje IF zanke
    }
  //branje senzorja DHT11:
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(WiFi.localIP());
  Serial.print(F(" IP address.   Humidity: "));
  Serial.print(h);
  Serial.print(F(" %.   "));
  Serial.print(F("T_REF: "));
  Serial.print(t_REF);
  Serial.print(F(" °C. "));
  Serial.print(F("  T_NOW: "));
  Serial.print(t);
  Serial.print(F(" °C. "));
  Serial.print(F("   Sensor IR: "));
  Serial.println(digitalRead(IR));
  
  /*--------------signalization--board-LED--WIFI---Connected---------------------*/
    if (WiFi.status()==WL_CONNECTED) {
    digitalWrite(LED_ESP,HIGH);   //BLUE LED on ESP shows of status WiFi!
    }
    else{
      digitalWrite(LED_ESP,LOW);   //BLUE LED on ESP shows of status WiFi!
      }

  //ČE TEMP presega REF + dT - stanje kuhanja!
  if (t > t_REF + dT) {
    digitalWrite (GREEN, HIGH);
    delay(100);
    digitalWrite (GREEN, LOW);
    digitalWrite (BLUE, LOW);
    digitalWrite (RED, LOW);
    fire_on_counter++;
    if (fire_on_counter > 12*hot_min) {     //določiti število ciklov, ko se sproži alarm! (1 cikel = 5 sekund)
      for (int j = 0; j <=4; j++) {         //ponovitev 3x alarm
        digitalWrite (GREEN, LOW);
        toggleLED(RED);                     //vklop/izklop rdeče LED
        delay(1000);
        for (int i = 0; i <= 45; i++) {     //loopanje za 1 sek alarm
          if (digitalRead(IR)==LOW) {        //omogočanje piskanje alarma le, ko ni osebe
            digitalWrite(BUZZ, HIGH);
            delay(10);
            digitalWrite(BUZZ, LOW);
            }
          delay(10);
          }
        }
      //izstop iz FOR zanke za alarmiranje.
      fire_on_counter = 0;
    }
  }
  //temp ne presega REF + dT - ugasnjen gorilnik
  else {
    digitalWrite (BLUE, HIGH);
    delay(100);
    digitalWrite(BLUE,LOW);
    digitalWrite (GREEN, LOW);
    fire_off_counter++;
    if (fire_off_counter > 12*cold_min) {   //določiti število ciklov, ko je štedilnik hladen, preden se izklopi Cooker-check.
      //izklop naprave (deep sleep)
      digitalWrite (GREEN, HIGH);
      digitalWrite (BLUE, HIGH);
      digitalWrite (RED, HIGH);
      delay(1500);
      Serial.println("Turning into DEEP SLEEP");
      esp_deep_sleep_start();               //izklop naprave (se samodejno zbudi kdaj!)
      delay(5000);
      }
    }
    // turn on the LED
  delay(4900); //orig. naj bo 4900 - med meritvami vsaj 5 s razlike! 
}
