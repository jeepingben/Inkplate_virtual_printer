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
#define IMGBUFSIZE 65535
#define HTMLBUFSIZE 8192

RTC_DATA_ATTR int page = 1;
RTC_DATA_ATTR int lastpage = 99;

SdFile file;
Inkplate display(INKPLATE_1BIT);

const char *ssid = "Maine Volcano Observatory";
const char *password = "Eufm-Qmp2-rzrp-AgaL";
char url[46]; // "https://jeepingben.net/epaper-bmps/pagexx.png"

byte touchPadPin = 10;

void annotate() {
  display.setTextColor(0, 7);
  display.setTextSize(1);
  display.setCursor(10, 0);
  display.rtcGetRtcData();  
    display.print(display.readBattery(), 2); // Print battery voltage
    display.printf("V %dC %02d:%02d",display.readTemperature(), display.rtcGetHour(), display.rtcGetMinute());
   
  display.setTextSize(3);
    
    display.setCursor(10, 480);
    display.printf("%d", page == 1?1:page - 1);
    display.setCursor(10, 580); 
    display.print('0');         
    display.setCursor(10, 680); 
    display.printf("%d", page == lastpage?lastpage:page + 1);         
}
void setup()
{

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
    display.preloadScreen();
    display.setRotation(3);
    
    display.clearDisplay();
    display.fillCircle(random(500), random(500), 50, BLACK);
    display.partialUpdate(true);
    //display.display();
    

  


  if (display.rtcCheckAlarmFlag()) {
    display.rtcClearAlarmFlag();
    drawxkcd(NEWESTXKCD); // Check day of week to see if it is an xkcd day?
    gotosleep();
  }
  display.rtcSetAlarm(99 /*sec*/, 99 /*min*/, 3 /*hour*/, 99 /*day*/, 99 /*weekday*/);
    
    if(padStatus == 7) {
      drawxkcd(RANDOMXKCD);
      gotosleep();
    } 

    if (!display.sdCardInit()) {
       display.println("Failed to initialize SD card");
       display.partialUpdate();
       drawxkcd(RANDOMXKCD);
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
      
      getPages();
       
      gotosleep();

}

void getPages() {
  int32_t imglen;
  uint8_t* imgbuffer;
  imgbuffer = (uint8_t*)malloc(IMGBUFSIZE * sizeof(uint8_t));
  if (imgbuffer == NULL) {
    display.println("Malloc for page buffer failed");
    display.display();
    return;
  }
  lastpage=1;
    file.rmRfStar();
    do {
        imglen = 0;
        sprintf(url, "https://jeepingben.net/epaper-bmps/page%d.png", page);
        imglen = loadhttp(url, imgbuffer, IMGBUFSIZE);
        
        
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
          if (lastpage == 1) {
              showPage(lastpage);
              //drawing PNG from the buffer isn't available see drawBitmapFromBuffer
              //display.drawImage(imgbuffer,imglen, 0, 0, false, false);
          }
          lastpage++;
        }
           
    } while(imglen > 0);
    lastpage -= 1;
    free(imgbuffer);
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
    char *strptr;
    uint8_t* imgbuffer;
    const char imganchor[]="<img id=\"comic\" src=\"";
    const char altanchor[]="<p id=\"altText\">";
    const char htmlcode[] = "&#39;"; // There are many apostrophes that make alt text hard to read
    char xkcdurl[80] = "https://m.xkcd.com";// reused later as "https:";
    imgbuffer = (uint8_t*)malloc(HTMLBUFSIZE * sizeof(uint8_t));
    if (imgbuffer == NULL) {
      display.println("Malloc for html buffer failed");
      display.partialUpdate();
    return;
  }
    wifiup();

    if (mode == RANDOMXKCD) {
      char rnd[8];
      sprintf(rnd,"/%d/", random(2500));
      strcat(xkcdurl, rnd);
    }
    loadhttp(xkcdurl, imgbuffer, HTMLBUFSIZE);
    strcpy(xkcdurl, "https:");
    // parse out imgurl
    strmatch = strstr((char*)imgbuffer,imganchor );
    if (strmatch == NULL) {
      display.println("Failed to find comic image in downloaded html");
      display.partialUpdate();
      free(imgbuffer);
      return;
    }
    strmatch += strlen(imganchor);

    strend = strstr(strmatch, "\"");
    if (strend == NULL || strend - strmatch >= 80) {
        display.println("Failed to parse comic image url");
        display.partialUpdate();
        free(imgbuffer);
        return;
    }
    strncat(xkcdurl, strmatch, (strend - strmatch));

    display.selectDisplayMode(INKPLATE_3BIT);
    display.setTextColor(0, 7);
    
    //position is offset significantly to the right with this call as of 3/22
    //display.drawImage(xkcdurl, Image::Format::PNG, Image::Position::Center, false, false);
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
        *strend = '\0';
        display.setTextSize(3);

        // un-escape apostrophtes (modifies alt-text buffer)
        strptr = strstr(strmatch, htmlcode );
        while (strptr != NULL) {
          strptr[0] = '\0';
          display.printf("%s'", strmatch);
          strmatch = strptr + 5; //Length of htmlcode 
          strptr = strstr(strmatch, htmlcode );
        }
        display.print(strmatch);
    }
    free(imgbuffer);
    annotate();
    display.display();
}

void gotosleep() {
    WiFi.mode(WIFI_OFF);
   
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

int32_t loadhttp(char* url, uint8_t* imgbuffer, int32_t maxsize) {
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
          res = http.getStreamPtr()->readBytes(imgbuffer, ((len > maxsize) ? maxsize : len));  
        }
    }
    http.end();
    return res;
}

void wifiup() {

    uint8_t attempts = 0;
    display.setTextSize(5);
    display.print("Connecting to WiFi...");
    display.partialUpdate();

    display.partialUpdate();

    // Connect to the WiFi network.
    WiFi.mode(WIFI_MODE_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500); //todo brd can this be a timer wait?
        display.print(".");
        display.partialUpdate();
    }
    if (WiFi.status() != WL_CONNECTED) {
        display.print("Failed to connect to wifi");
        display.partialUpdate();
        gotosleep();
    }
    display.println("\nWiFi OK! Downloading...");
    display.partialUpdate();
}


void loop()
{
    // Nothing...
}
