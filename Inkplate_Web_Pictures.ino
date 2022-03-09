/*
  Download and display pages from the virtual printer on jeepingben.net
  load xkcd overnight
*/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE10
#error "Wrong board selection for this example, please select Inkplate 10 in the boards menu."
#endif

#include "HTTPClient.h"          //Include library for HTTPClient
#include "Inkplate.h"            //Include Inkplate library to the sketch
#include "WiFi.h"                //Include library for WiFi

RTC_DATA_ATTR int page = 1;

Inkplate display(INKPLATE_1BIT); // Create an object on Inkplate library and also set library into 1 Bit mode (BW)

const char ssid[] = "Maine Volcano Observatory";    // Your WiFi SSID
const char *password = "Eufm-Qmp2-rzrp-AgaL"; // Your WiFi password

byte touchPadPin = 10;
byte padStatus = 0;
void setup()
{
    reason == timer
      drawxkcd
      wifidown
      sleep

    
    other
    filesystem init

    padStatus &= display.readTouchpad(PAD1);
    padStatus &= (display.readTouchpad(PAD2) << 1);
    padStatus &= (display.readTouchpad(PAD3) << 2);

    if (padStatus & 1) { //pad1
      if (page > 1) {
          page--;
          showpage(page);
          gotosleep();
      }
    }
    if (padStatus & 4) { // pad3
          page++;
          showpage(page);
          gotosleep();
    }
    
    // Pad2 or power connected, etc
      page=1;
      wifiup();
      download page1
        display
        save
      download other pages
      wifi off
      gotosleep();
    
      
    int temperature;
    float voltage;
    
    display.begin();        // Init Inkplate library (you should call this function ONLY ONCE)
    display.clearDisplay(); // Clear frame buffer of display
    display.display();      // Put clear image on display
    display.setTextSize(5); 
    display.print("Connecting to WiFi...");
    display.partialUpdate();
    temperature = display.readTemperature(); // Read temperature from on-board temperature sensor
    voltage = display.readBattery(); 
    display.print(voltage, 2); // Print battery voltage
    display.print('V');
    display.print(temperature, DEC); // Print temperature
    display.print('C');
    display.partialUpdate();

    // Connect to the WiFi network.
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500); //todo brd can this be a timer wait?
        display.print(".");
        display.partialUpdate();
    }
    display.println("\nWiFi OK! Downloading...");
    display.partialUpdate();
    display.selectDisplayMode(INKPLATE_3BIT);

    // Draw the second image from web, this time using a HTTPClient to fetch the response manually.
    // Full color 24 bit images are large and take a long time to load, will take around 20 secs.
    HTTPClient http;
    // Set parameters to speed up the download process.
    http.getStream().setNoDelay(true);
    http.getStream().setTimeout(1);
     

    if (!display.drawImage("https://jeepingben.net/epaper-bmps/page1.png", 0, 0, false, false))
    {
        // If is something failed (wrong filename or format), write error message on the screen.
        display.println("Image open error");
        display.display();
    }
    display.display();
    
    http.end();

    WiFi.mode(WIFI_OFF);

   
    gotosleep();
}

void anotate() {
    display.setCursor(480, 790); // Set new print position (right above first touchpad)
    display.print('-');          // Print minus sign
    display.setCursor(580, 790); // Set new print position (right above second touchpad)
    display.print('0');          // Print zero
    display.setCursor(680, 790); // Set new print position (right above third touchpad)
    display.print('+');          // Print plus sign
}
void gotosleep() {

   // Get current time
   // set timer for xkcdtime - currenttime seconds
    // Setup mcp interrupts
    display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, touchPadPin, INPUT);
    display.setIntOutputInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 1, false, false, HIGH);
    display.setIntPinInternal(MCP23017_INT_ADDR, display.mcpRegsInt, touchPadPin, RISING);

    
    // Enable wakup from deep sleep on gpio 36
    //esp_sleep_enable_ext1_wakeup((1ULL << 36), ESP_EXT1_WAKEUP_ALL_LOW);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1);
    esp_deep_sleep_start();
}

void wifiup () {
  
    int temperature;
    float voltage;
    display.setTextSize(5); 
    display.print("Connecting to WiFi...");
    display.partialUpdate();
    temperature = display.readTemperature(); // Read temperature from on-board temperature sensor
    voltage = display.readBattery(); 
    display.print(voltage, 2); // Print battery voltage
    display.print('V');
    display.print(temperature, DEC); // Print temperature
    display.print('C');
    display.partialUpdate();

    // Connect to the WiFi network.
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500); //todo brd can this be a timer wait?
        display.print(".");
        display.partialUpdate();
    }
    display.println("\nWiFi OK! Downloading...");
    display.partialUpdate();
}


void loop()
{
    // Nothing...
}