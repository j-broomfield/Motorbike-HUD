//#include <WiFi.h>
#include <WiFiMulti.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <SFE_BMP180.h>

SFE_BMP180 bmp180;
const double locationAltitudeMeters = 155.0; // Altitude of current location in meters
float Po = 1015.0;
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h> //OLED display library
#include <Adafruit_SSD1305.h> //OLED display library
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans18pt7b.h>

#include <RTClib.h>

#include <EEPROM.h> //This library allows reading and writing to the EEPROM
#define OLED_RESET -1

//Set wifi defaults here
const char *ssid2 = "1"; // your wifi name
const char *pw2   = "1";// your wifi password

const char *ssid1 = "1"; // your wifi name
const char *pw1   = "1";// your wifi password

const char *ssid3 = "1"; // your wifi name
const char *pw3   = "1";// your wifi password

//Hotspot
const char *ssid4 = "PHONE"; // your wifi name
const char *pw4   = "12345678";// your wifi password

const long timezoneOffset = 1 * 60 * 60; // ? hours * 60 * 60

//ESP32Time rtc(0);
RTC_DS3231 rtc;
char t[32];

//Buttons

const int buttonPinUp = 16;
int buttonUpState = 0;
int upBtn = 1;
const int buttonPinDown = 19;
int buttonDownState = 0;
int downBtn = 2;
const int buttonPinSelect = 17;
int buttonSelectState = 0;
int selectBtn = 3;

const unsigned long selectHoldTime = 2000;
bool selectHoldTriggered = false;

const char          *ntpServer  = "uk.pool.ntp.org"; // change it to local NTP server if needed
const unsigned long updateDelay = 900000;         // update time every 15 min
const unsigned long retryDelay  = 5000;           // retry 5 sec later if time query failed
const String        weekDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
String fTime;

String weatherTrendIcon = "-";

double previousSeaLevelPressure = 0;
unsigned long lastPressureTrendCheck = 0;
const unsigned long pressureTrendCheckDelay = 300000; // 5 minutes

const double pressureTrendThreshold = 1.0; // hPa change needed to count as rising/falling

unsigned long lastUpdatedTime = updateDelay * -1;
unsigned int  second_prev = 0;
bool          colon_switch = false;
bool          wifiConnected = false;
bool          timeSet;
bool          forceNumberVisible;

int           CurrentScreen = 0; //Home default

bool          timeAlterMode = false; //default
int           timeAlterPosition = 0; //0=hour 1=min

WiFiMulti WiFiMulti; 
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer);
//Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);
Adafruit_SSD1305 display(128, 32, &Wire, OLED_RESET);


#define HONDA_2_HEIGHT 32
#define HONDA_2_WIDTH 128

#define WEATHER_ICON_WIDTH 12
#define WEATHER_ICON_HEIGHT 12
#define WEATHER_ICON_X 105
#define WEATHER_ICON_Y 17

const unsigned char weather_sun_bmp [] PROGMEM = {
  0x09, 0x00,
  0x49, 0x20,
  0x20, 0x40,
  0x0F, 0x00,
  0xD0, 0xB0,
  0x10, 0x80,
  0x10, 0x80,
  0xD0, 0xB0,
  0x0F, 0x00,
  0x20, 0x40,
  0x49, 0x20,
  0x09, 0x00,
};

const unsigned char weather_cloud_bmp [] PROGMEM = {
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x01, 0xC0,
  0x1A, 0x20,
  0x64, 0x20,
  0x80, 0x10,
  0x80, 0x10,
  0x7F, 0xE0,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
};

const unsigned char weather_rain_bmp [] PROGMEM = {
  0x00, 0x00,
  0x01, 0xC0,
  0x1A, 0x20,
  0x64, 0x20,
  0x80, 0x10,
  0x80, 0x10,
  0x7F, 0xE0,
  0x00, 0x00,
  0x24, 0x90,
  0x49, 0x20,
  0x92, 0x40,
  0x00, 0x00,
};
 
const unsigned char Honda_2_bmp [] PROGMEM = {
//static const unsigned char Honda_2_bmp[] u8x8_PROGMEM= {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x01, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x0F, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x3F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x07, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x1F, 0xFF, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xFF, 0xFF, 0xC3, 0xFF, 0x7F, 0xE3, 0xFF, 0xC7, 0xFE, 0x7F, 0xFF, 0xFF, 0x83, 0xFF, 0x00,
0x03, 0xFF, 0xFF, 0xC3, 0xFF, 0x7F, 0xEF, 0xFF, 0xF7, 0xFE, 0x7F, 0xFF, 0xFF, 0xC3, 0xFF, 0x80,
0x1F, 0xFF, 0xFF, 0x80, 0xFC, 0x0F, 0x9F, 0xC7, 0xF9, 0xFF, 0x1E, 0x1F, 0xFF, 0xE0, 0xFF, 0xC0,
0x7F, 0xFF, 0xFF, 0x80, 0x7C, 0x0F, 0x9F, 0x81, 0xF9, 0xFF, 0x8E, 0x1F, 0x03, 0xF0, 0xFF, 0xC0,
0x7F, 0xFF, 0xFC, 0x00, 0x7F, 0xFF, 0x9F, 0x80, 0xF9, 0xFF, 0xCE, 0x1F, 0x03, 0xF1, 0xE7, 0xE0,
0x7F, 0xFF, 0xFE, 0x00, 0x7F, 0xFF, 0x9F, 0x00, 0xF9, 0xEF, 0xCE, 0x1F, 0x03, 0xF1, 0xFF, 0xF0,
0x7F, 0xFF, 0xFC, 0x00, 0x7F, 0xFF, 0x9F, 0x80, 0xF9, 0xE7, 0xEE, 0x1F, 0x03, 0xF3, 0xFF, 0xF0,
0x3F, 0xFF, 0xF0, 0x00, 0x7C, 0x0F, 0x9F, 0x81, 0xF9, 0xE7, 0xFE, 0x1F, 0x03, 0xF7, 0xFF, 0xF8,
0x1F, 0xFF, 0x80, 0x00, 0xFC, 0x0F, 0x9F, 0xC3, 0xF9, 0xE3, 0xFE, 0x1F, 0x0F, 0xE7, 0x81, 0xFC,
0x0F, 0xFF, 0xC0, 0x03, 0xFF, 0x7F, 0xEF, 0xFF, 0xF7, 0xF9, 0xFE, 0x7F, 0xFF, 0xFF, 0xC3, 0xFF,
0x07, 0xFF, 0xF0, 0x03, 0xFF, 0x7F, 0xE7, 0xFF, 0xE7, 0xF9, 0xFE, 0x7F, 0xFF, 0xBF, 0xC3, 0xFF,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
}; 
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
 
// ------------------- For i2c -------------------
//// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
  

void setup() {

  Serial.begin(9600);
  //Draw splash screen
  yield();
  display.begin(0x3C);
  yield();
  display.clearDisplay();

  // init done
  pinMode(buttonPinUp, INPUT_PULLUP);
  pinMode(buttonPinDown, INPUT_PULLUP);
  pinMode(buttonPinSelect, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  //Bitmap on screen
  display.drawBitmap(0,1, Honda_2_bmp, HONDA_2_WIDTH, HONDA_2_HEIGHT, 1);
  display.display();
  delay(5000);

  drawWifiTxtToDisplay(" Connecting --to Wifi--");
  rtc.begin();
  bool success = bmp180.begin();

  if (success) {
    Serial.println("BMP180 init success");
  }

  //Saved Wifi logins
  WiFiMulti.addAP(ssid1, pw1); // multiple ssid/pw can be added
  WiFiMulti.addAP(ssid2, pw2); // multiple ssid/pw can be added
  WiFiMulti.addAP(ssid3, pw3); // multiple ssid/pw can be added
  WiFiMulti.addAP(ssid4, pw4); // multiple ssid/pw can be added  



  for (int i = 0; i < 2; i++){
    if (WiFiMulti.run() == WL_CONNECTED) {
      wifiConnected = true;
      drawWifiTxtToDisplay("...Wifi....  ..connected.");
      delay(1500);

      drawWifiTxtToDisplay("..Updating......RTC....");
      delay(2000);

      break;
    }
    if (WiFiMulti.run() != WL_CONNECTED) {
      delay(200);
      Serial.print(".");
    }
  }
  
  if (wifiConnected == true) {
    Serial.println("\nConnected.");
    timeClient.setTimeOffset(timezoneOffset);
    timeClient.begin();
    timeClient.update();
    timeSet = true;

  }else{
    drawWifiTxtToDisplay("Connection  failed");
    delay(2000);
     
    drawWifiTxtToDisplay("HOTSPOT =    PHONE");
      delay(5000);
      
     drawWifiTxtToDisplay("PASSWORD=  12345678");
      delay(5000); 

    timeSet = false;
  }
}

void loop() { 
  //Button monitoring and logic
  forceNumberVisible = false;  
  int btnPressed = -1;
  btnPressed = ReadButtonState();
  if (btnPressed != -1){
    buttonLogic(btnPressed);
  }

//TODO turn into function rather tehn code in the loop
  //Get temperature, pressure, altitude from bosch sensor
  char status;
  double T, P, seaLevelPressure;
  bool success = false;
  int tempC;
  String strTempC = "";

  status = bmp180.startTemperature();
  if (status != 0) {
    status = bmp180.getTemperature(T);
    tempC = (int)T;
    strTempC = String(tempC) + " C";
    Serial.println(T);
  }

  if (status != 0) {
    status = bmp180.startPressure(3);
    if (status != 0) {
      status = bmp180.getPressure(P, T);
      if (status != 0) {
        seaLevelPressure = bmp180.sealevel(P, locationAltitudeMeters);
        updateWeatherTrend(seaLevelPressure);
      }
    }
  }

//Start Screen1
  //Dsiplay Home screen
  display.clearDisplay();
  //Time
  String currentTime = getTime(forceNumberVisible);
  drawTimeToDisplay(currentTime);

  //Temp
  drawTempToDisplay(strTempC);

  //Weather trend
  drawWeatherTrendToDisplay(weatherTrendIcon);

  // draws hor & ver lines for seperation of other values
  display.drawFastVLine(93,0,32,1);

  // draws hor & ver lines for seperation of other values
  display.drawFastVLine(93,0,32,1);
  display.drawFastHLine(93,15,37,1);
  display.drawRect(0,0,128,32, 1);
//End Screen1

  //only display done during loop, prevents flashing of ui
  display.display();
}

void buttonLogic(int btnPressed){
  if (CurrentScreen == 0){
    forceNumberVisible = true;
    if (btnPressed == selectBtn){
      if (!timeAlterMode && wifiConnected == false && selectHoldTriggered){
        timeAlterMode = true;
        timeAlterPosition = 0;
      } else if (timeAlterMode && timeAlterPosition == 0){
        timeAlterPosition++;
      } else if (timeAlterMode && timeAlterPosition == 1){
        timeAlterPosition = 0;
        timeAlterMode = false;
        //Set time here
      }
    } else if (btnPressed == upBtn){
      if (timeAlterMode){
        //Keep display updated
        drawTimeToDisplay(fTime);
        DateTime now = rtc.now();
        if (timeAlterPosition == 0){
          //Hour++
          int currentHour = now.hour();
          if (currentHour < 23 ){
            currentHour++;
          } else {
            currentHour = 0;
          }
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), currentHour, now.minute(), now.second()));
        } else {
          //Min++
          int currentMin = now.minute();
          if (currentMin < 59 ){
            currentMin++;
          } else {
            currentMin = 0;
          }
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), currentMin, now.second()));
        }
      }
    } else if (btnPressed == downBtn){
      if (timeAlterMode){
        //Keep display updated
        drawTimeToDisplay(fTime);
        DateTime now = rtc.now();
         if (timeAlterPosition == 0){
          //Hour--
          int currentHour = now.hour();
          if (currentHour > 0 ){
            currentHour--;
          } else {
            currentHour = 23;
          }
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), currentHour, now.minute(), now.second()));
        } else {
          //Min--
          int currentMin = now.minute();
          if (currentMin > 0 ){
            currentMin--;
          } else {
            currentMin = 59;
          }
          rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), currentMin, now.second()));
        }
      }
    }
    
  }
}

String getTime(bool forceNumbersVisible){
  //Grabs time from internet or from RTC

  //Internet
  if (wifiConnected == true){

    time_t rawtime = timeClient.getEpochTime();
    struct tm * ti;

    ti = localtime (&rawtime);
    unsigned int year = ti->tm_year + 1900;

    ti = localtime (&rawtime);
    unsigned int month = ti->tm_mon + 1;

    ti = localtime (&rawtime);
    unsigned int day = ti->tm_mday;

    unsigned int hour = timeClient.getHours();
    unsigned int minute = timeClient.getMinutes();
    unsigned int seconds = timeClient.getSeconds();

    rtc.adjust(DateTime(year, month, day, hour, minute, seconds));
    wifiConnected = false;  
    timeClient.end(); 
  }
  
  //Time string creation from RTC
  DateTime now = rtc.now();
  sprintf(t, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());  
  unsigned long t = millis();
  int second = now.second();
  if (second != second_prev) colon_switch = !colon_switch;
  
  //Keep time visible for a second when changing the time
  if (forceNumbersVisible){
    colon_switch = true;
  }

  String hour = (now.hour() < 10 ? "0" : "") + String(now.hour());
  String min = (now.minute() < 10 ? "0" : "") + String(now.minute());
  
  if (timeAlterMode && !forceNumbersVisible){//Flash selected number for altering
    if (timeAlterPosition == 0){ //Hour
      hour = (colon_switch ? hour : "    ");
    } else { // Assume mins
      min = (colon_switch ? min : "    ");
    }
  }
  
  fTime = hour + (colon_switch ? ":" : " ") + min;
  second_prev = second;

  return fTime;
}

void drawWifiTxtToDisplay(String txt){
  //21 char per line
  display.setFont(&FreeMono9pt7b);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextWrap(true);
  display.setTextColor(WHITE);
  display.setCursor(1,13);
  display.print(txt);
  //Serial.println(txt);
  display.display();
}
void drawTimeToDisplay(String txt){
  display.setFont(&FreeSans18pt7b);
  display.setTextSize(1);
  display.setTextWrap(true);
  display.setTextColor(WHITE);
  display.setCursor(3,28);
  //Serial.println(txt);
  display.print(txt);

}

void drawTempToDisplay(String txt){
  //display.setFont(&FreeMono9pt7b);
  display.setFont();
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setTextColor(WHITE);
  display.setCursor(98,4);
  //Serial.println(txt);
  display.print(txt);
}

void drawWeatherTrendToDisplay(String txt){
  if (txt == "^") {
    display.drawBitmap(
      WEATHER_ICON_X,
      WEATHER_ICON_Y,
      weather_sun_bmp,
      WEATHER_ICON_WIDTH,
      WEATHER_ICON_HEIGHT,
      1
    );
  } else if (txt == "v") {
    display.drawBitmap(
      WEATHER_ICON_X,
      WEATHER_ICON_Y,
      weather_rain_bmp,
      WEATHER_ICON_WIDTH,
      WEATHER_ICON_HEIGHT,
      1
    );
  } else {
    display.drawBitmap(
      WEATHER_ICON_X,
      WEATHER_ICON_Y,
      weather_cloud_bmp,
      WEATHER_ICON_WIDTH,
      WEATHER_ICON_HEIGHT,
      1
    );
  }
}

void updateWeatherTrend(double seaLevelPressure){
  if (previousSeaLevelPressure == 0) {
    previousSeaLevelPressure = seaLevelPressure;
    lastPressureTrendCheck = millis();
    weatherTrendIcon = "-";
    return;
  }

  if (millis() - lastPressureTrendCheck >= pressureTrendCheckDelay) {
    double pressureChange = seaLevelPressure - previousSeaLevelPressure;

    if (pressureChange > pressureTrendThreshold) {
      weatherTrendIcon = "^";
    } else if (pressureChange < -pressureTrendThreshold) {
      weatherTrendIcon = "v";
    } else {
      weatherTrendIcon = "-";
    }

    previousSeaLevelPressure = seaLevelPressure;
    lastPressureTrendCheck = millis();
  }
}

int ReadButtonState(){
  const unsigned long debounceDelay = 200;
  static unsigned long lastButtonPressTime = 0;
  static unsigned long selectPressedStartTime = 0;
  static bool selectWasPressed = false;

  buttonUpState = digitalRead(buttonPinUp);
  buttonDownState = digitalRead(buttonPinDown);
  buttonSelectState = digitalRead(buttonPinSelect);

  // Select button handling
  if (buttonSelectState == LOW) {
    if (!selectWasPressed) {
      selectWasPressed = true;
      selectPressedStartTime = millis();
      selectHoldTriggered = false;
    }

    // Only require a long hold when NOT already in time alter mode
    if (!timeAlterMode && !selectHoldTriggered && millis() - selectPressedStartTime >= selectHoldTime) {
      selectHoldTriggered = true;
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("select held");
      lastButtonPressTime = millis();
      return selectBtn;
    }

    return -1;
  }

  // Select button released
  if (selectWasPressed) {
    selectWasPressed = false;

    // If it was a long hold, do not also count the release as a tap
    if (selectHoldTriggered) {
      selectHoldTriggered = false;
      digitalWrite(LED_BUILTIN, LOW);
      return -1;
    }

    // Once already editing, a normal tap should advance/set the time
    if (timeAlterMode && millis() - lastButtonPressTime >= debounceDelay) {
      Serial.println("select pressed");
      digitalWrite(LED_BUILTIN, HIGH);
      lastButtonPressTime = millis();
      return selectBtn;
    }
  }

  if (millis() - lastButtonPressTime < debounceDelay) {
    return -1;
  }

  if (buttonUpState == LOW){
    Serial.println("up pressed");
    digitalWrite(LED_BUILTIN, HIGH);
    lastButtonPressTime = millis();
    return upBtn;
  }

  if (buttonDownState == LOW){
    Serial.println("down pressed");
    digitalWrite(LED_BUILTIN, HIGH);
    lastButtonPressTime = millis();
    return downBtn;
  }

  digitalWrite(LED_BUILTIN, LOW);
  return -1;
}