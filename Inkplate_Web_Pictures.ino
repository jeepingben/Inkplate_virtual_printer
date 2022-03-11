/*
  Download and display pages from the virtual printer on jeepingben.net
  load xkcd overnight
*/

#include "Inkplate.h"
#include "WiFi.h"
#include "SdFat.h"
#include "HTTPClient.h"

#define NEWESTXKCD 0
#define RANDOMXKCD 1

RTC_DATA_ATTR int page = 1;
SdFile file;
Inkplate display(INKPLATE_1BIT);

const char *ssid = "Maine Volcano Observatory";
const char *password = "Eufm-Qmp2-rzrp-AgaL";
uint8_t imgbuffer[15535];
char url[46]; // "https://jeepingben.net/epaper-bmps/pagexx.png"

byte touchPadPin = 10;

void annotate() {
  display.setTextColor(0, 7);
    display.setCursor(10, 480);
    if (page != 1) {
       display.printf("%d", page - 1);
    }
    display.setCursor(10, 580); 
    display.print('0');         
    display.setCursor(10, 680); 
    display.printf("%d", page + 1);         
}
void setup()
{
    int32_t imglen;
    byte padStatus = 0;

    // Do just enough init to read pad status
    Wire.begin();
    display.digitalWriteInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 9, HIGH);
    display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, PAD1, INPUT);
    display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, PAD2, INPUT);
    display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, PAD3, INPUT);
    padStatus |= display.readTouchpad(PAD1);
    padStatus |= display.readTouchpad(PAD2) << 1;
    padStatus |= display.readTouchpad(PAD3) << 2;

    // Show the user we're awake
    display.begin();
    display.setRotation(3);
    display.clearDisplay();
    display.display();
    
  display.rtcClearAlarmFlag();  // Clear alarm flag from any previous alarm
  display.rtcSetAlarm(99 /*sec*/, 99 /*min*/, 3 /*hour*/, 99 /*day*/, 99 /*weekday*/);
  if (!display.rtcIsSet())      // Check if RTC is already is set. If ts not, set time and date
  {
    //  setTime(hour, minute, sec);
    display.rtcSetTime(7, 26, 00);
    //  setDate(weekday, day, month, yr);
    display.rtcSetDate(4, 10, 3, 2022); 

  }

    
    

    if (1) { //reason == timer
      drawxkcd(RANDOMXKCD);
      gotosleep();
    } else {}    //other

    if (!display.sdCardInit()) {
       display.println("Failed to initialize SD card");
       display.partialUpdate();
       //drawxkcd();
       gotosleep();
    }

    if ((padStatus & 1) && page > 1) { //pad1
          page--;
    }
    if ((padStatus & 4) && page < 99) { // pad3
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
              annotate();
              display.display();
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

void drawxkcd(uint8_t mode) {
    char *strmatch;
    char *strend;
    const char* imganchor="<img id=\"comic\" src=\"";
    const char* altanchor="<p id=\"altText\">";
    char xkcdurl[80] = "https://m.xkcd.com";// reused later as "https:";
    wifiup();

    if (mode == RANDOMXKCD) {
      char rnd[8];
      sprintf(rnd,"/%d/", random(2500));
      strcat(xkcdurl, rnd);
    }
    loadhttp(xkcdurl);
    strcpy(xkcdurl, "https:");
    // parse out imgurl
    strmatch = strstr((char*)imgbuffer,imganchor );
    if (strmatch == NULL) {
      display.println("Failed to find comic image in downloaded html");
      return;
    }
    strmatch += strlen(imganchor);
    
    strend = strstr(strmatch, "\"");
    if (strend == NULL || strend - strmatch >= 80) {
        display.println("Failed to parse comic image url");
        return;
    }
    strncat(xkcdurl, strmatch, (strend - strmatch));

    display.selectDisplayMode(INKPLATE_3BIT);
    display.setTextColor(0, 7);
    display.drawImage(xkcdurl, 20, 100, false, false);

    display.setCursor(10,1000);
    
    strmatch = strstr((char*)imgbuffer,altanchor );
    if (strmatch == NULL) {
      display.println("No alt-text today (parse error)");
    } else {
        strmatch += strlen(altanchor);
      
        strend = strstr(strmatch, "</p>");
        if (strend == NULL) {
            display.println("No alt-text today (terminator not found)");
        }
        display.setTextSize(3);
        display.printf("%.*s", strend - strmatch, strmatch);
    }
    annotate();
    display.display();

}

void gotosleep() {
    WiFi.mode(WIFI_OFF);
   // Get current time
   // set timer for xkcdtime - currenttime seconds
    // Setup mcp interrupts
    if (page != 1) {
        display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, PAD1, INPUT);
        display.setIntOutputInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 1, false, false, HIGH);
        display.setIntPinInternal(MCP23017_INT_ADDR, display.mcpRegsInt, PAD1, RISING);
    }
    for (int touchPadPin = 11; touchPadPin <=12; touchPadPin++) {
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
           display.print("finished stream read");
           display.partialUpdate();
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
