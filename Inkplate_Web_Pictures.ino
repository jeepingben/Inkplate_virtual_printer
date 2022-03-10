/*
  Download and display pages from the virtual printer on jeepingben.net
  load xkcd overnight
*/

#include "Inkplate.h"
#include "WiFi.h"
#include "SdFat.h"
#include "HTTPClient.h"

RTC_DATA_ATTR int page = 1;
SdFile file;
Inkplate display(INKPLATE_1BIT);

const char *ssid = "Maine Volcano Observatory";
const char *password = "Eufm-Qmp2-rzrp-AgaL";
uint8_t imgbuffer[65535];
char url[46]; // "https://jeepingben.net/epaper-bmps/pagexx.png"

byte touchPadPin = 10;

void annotate() {
  display.setTextColor(0, 7);
    display.setCursor(10, 480); 
    display.printf("%d", page - 1);    
    display.setCursor(10, 580); 
    display.print('0');         
    display.setCursor(10, 680); 
    display.printf("%d", page + 1);         
}
void setup()
{
    int32_t imglen;
    uint8_t padStatus = 0;  
    display.digitalWriteInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 9, HIGH);
    display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, PAD1, INPUT);
    display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, PAD2, INPUT);
    display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, PAD3, INPUT);
    
    if (display.readTouchpad(PAD1)) {
      padStatus != 1;
    }
    if (display.readTouchpad(PAD2)) {
      padStatus != 2;
    }
    if (display.readTouchpad(PAD3)) {
      padStatus |= 4;
    }
    display.begin();
    display.setRotation(3);
  
  display.rtcClearAlarmFlag();  // Clear alarm flag from any previous alarm
  display.rtcSetAlarm(99 /*sec*/, 99 /*min*/, 3 /*hour*/, 99 /*day*/, 99 /*weekday*/);
  if (!display.rtcIsSet())      // Check if RTC is already is set. If ts not, set time and date
  {
    //  setTime(hour, minute, sec);
    display.rtcSetTime(7, 26, 00); // 24H mode, ex. 6:54:00
    //  setDate(weekday, day, month, yr);
    display.rtcSetDate(4, 10, 3, 2022); // 0 for Sunday, ex. Saturday, 16.5.2020.

    // display.rtcSetEpoch(1589610300); // Or use epoch for setting the time and date
  }

    
    
    display.clearDisplay();
    display.display();
    if (0) { //reason == timer
      drawxkcd();
      gotosleep();
    } else {}    //other

    if (!display.sdCardInit()) {
       display.println("Failed to initialize SD card");
       display.partialUpdate();
       //drawxkcd();
       gotosleep();
    }
    
    
    
    display.print(padStatus,DEC);
    display.partialUpdate();
    if ((padStatus & (byte)1) && page > 1) { //pad1
          page--;
    }
    if ((padStatus & (byte)4) && page < 99) { // pad3
          page++;
    }

    if (!(padStatus & 2)) {
      showPage(page);
      gotosleep();
    }

    // Pad2 or power connected, etc
      page=1;
      wifiup();
      
      file.rmRfStar();
      do {
          imglen = 0;
          sprintf(url, "https://jeepingben.net/epaper-bmps/page%d.png", page);
          display.println(url);
          display.display();
          imglen = loadhttp(url);
          
          
          if (imglen != 0) {
          if (!file.open(&url[35], O_WRITE|O_CREAT)) {
            display.println("SDCard file open error");
            display.println(&url[35]);
            display.display();
            imglen = 0;
          } else {
            file.write(imgbuffer, imglen);
            file.flush();
            file.close();
          }
          if (page == 1) {
              showPage(page);
              //drawing PNG from the buffer isn't available see drawBitmapFromBuffer
              //display.drawImage(imgbuffer,imglen, 0, 0, false, false);
              //annotate();
              //display.display();
          }
          page++;
          }
             
      } while(imglen > 0);
      
       
      gotosleep();


/*
    display.selectDisplayMode(INKPLATE_3BIT);

    if (!display.drawImage("https://jeepingben.net/epaper-bmps/page1.png", 0, 0, false, false))
    {
        // If is something failed (wrong filename or format), write error message on the screen.
        display.println("Image open error");
        display.display();
    }
    display.display();
*/

}

void showPage(int pagenum) {
  char filename[12];
  sprintf(filename, "page%d.png", pagenum);

  display.selectDisplayMode(INKPLATE_3BIT);
  if (!display.drawImage(filename, 0, 0, false, false))
  {
        if (page > 1) {
          page--;
        }
        // If is something failed (wrong filename or format), write error message on the screen.
        display.print("Image open error - cannot open ");
        display.print(filename);
  }

  annotate();
  display.display();
}

void drawxkcd() {
    
    wifiup();
    // draw xkcd image
    // get alttext
    // write alttext
    annotate();
    display.display();
    WiFi.mode(WIFI_OFF);
}

void gotosleep() {
    WiFi.mode(WIFI_OFF);
   // Get current time
   // set timer for xkcdtime - currenttime seconds
    // Setup mcp interrupts
    for (int touchPadPin = 10; touchPadPin <=12; touchPadPin++) {
    display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, touchPadPin, INPUT);
    display.setIntOutputInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 1, false, false, HIGH);
    display.setIntPinInternal(MCP23017_INT_ADDR, display.mcpRegsInt, touchPadPin, RISING);
    }

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0); // RTC
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1); // touch
    esp_deep_sleep_start();
}

int32_t loadhttp(char* url) {
      int32_t res = 0;
        HTTPClient http;
      // Set parameters to speed up the download process.
      http.getStream().setNoDelay(true);
      http.getStream().setTimeout(1);
  http.begin(url);
  int httpCode = http.GET();
  if (httpCode == 200)
    {
        // Get the response length and make sure it is not 0.
        int32_t len = http.getSize();
        if (len > 0) {         
          res = http.getStreamPtr()->readBytes(imgbuffer, ((len > sizeof(imgbuffer)) ? sizeof(imgbuffer) : len));
          
        }
    }
    http.end();
          return res;
}
void wifiup() {

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
