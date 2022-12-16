/*
  Download and display pages from a virtual printer
  load xkcd overnight
*/

#include "Inkplate.h"
#include "WiFi.h"
#include "SdFat.h"
#include "HTTPClient.h"
#include "config.h"

#define NEWESTXKCD 0
#define RANDOMXKCD 1
#define IMGBUFSIZE 102400
#define HTMLBUFSIZE 8192

#define LOADING { \
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xff,0xf0,0x30,0x3f,0xff,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xfe,0x0,0x30,0x3,0xff,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xf0,0x0,0x30,0x0,0x7f,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xc0,0x0,0x30,0x0,0x1f,0xff,0xff,0xf0,\
0xff,0xff,0xff,0x0,0x0,0x30,0x0,0x7,0xff,0xff,0xf0,\
0xff,0xff,0xff,0x80,0x0,0x30,0x0,0x7,0xff,0xff,0xf0,\
0xff,0xff,0xf9,0x80,0x0,0x30,0x0,0xe,0x7f,0xff,0xf0,\
0xff,0xff,0xe1,0xc0,0x0,0x30,0x0,0xc,0x3f,0xff,0xf0,\
0xff,0xff,0xc0,0xc0,0x0,0x30,0x0,0x1c,0x1f,0xff,0xf0,\
0xff,0xff,0x80,0xe0,0x3,0xff,0x0,0x18,0x7,0xff,0xf0,\
0xff,0xff,0x0,0x60,0x3f,0xff,0xf0,0x38,0x3,0xff,0xf0,\
0xff,0xfe,0x0,0x71,0xff,0xff,0xfe,0x70,0x1,0xff,0xf0,\
0xff,0xfc,0x0,0x3f,0xff,0xff,0xff,0xe0,0x0,0xff,0xf0,\
0xff,0xf8,0x0,0x1f,0xff,0xff,0xff,0xe0,0x0,0x7f,0xf0,\
0xff,0xf0,0x0,0x3f,0xff,0xff,0xff,0xf0,0x0,0x7f,0xf0,\
0xff,0xe0,0x0,0xff,0xff,0xff,0xff,0xf8,0x0,0x3f,0xf0,\
0xff,0xe0,0x1,0xff,0xff,0xff,0xff,0xfc,0x0,0x1f,0xf0,\
0xff,0xc0,0x3,0xff,0xff,0xff,0xff,0xff,0x0,0xf,0xf0,\
0xff,0x80,0x7,0xff,0xff,0xff,0xff,0xff,0x80,0xf,0xf0,\
0xff,0xe0,0xf,0xff,0xff,0xff,0xff,0xff,0x80,0x1f,0xf0,\
0xff,0xf0,0x1f,0xff,0xff,0xff,0xff,0xff,0xc0,0x7f,0xf0,\
0xff,0x3c,0x3f,0xff,0xff,0xff,0xff,0xff,0xe1,0xf3,0xf0,\
0xfe,0x1f,0x3f,0xff,0xff,0xff,0xff,0xff,0xf7,0xc3,0xf0,\
0xfe,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x1,0xf0,\
0xfc,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x1,0xf0,\
0xfc,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x0,0xf0,\
0xf8,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x0,0xf0,\
0xf8,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x0,0xf0,\
0xf8,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x0,0x70,\
0xf8,0x3,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x0,0x70,\
0xf0,0x3,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x70,\
0xf0,0x3,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x70,\
0xf0,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x70,\
0xf0,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x30,\
0xf0,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x30,\
0xf0,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x30,\
0xe0,0x7,0xff,0xfc,0xff,0xff,0xff,0xcf,0xff,0x80,0x30,\
0xe0,0x7,0xff,0x7b,0x7b,0xb6,0xdd,0xbf,0xff,0x80,0x30,\
0xff,0xff,0xff,0x77,0xb3,0xba,0xcd,0x7f,0xff,0xff,0xf0,\
0xff,0xff,0xff,0x77,0xbd,0xba,0xd1,0x67,0xff,0xff,0xf0,\
0xe0,0xf,0xff,0x73,0x25,0xb6,0xd9,0x37,0xff,0x80,0x30,\
0xe0,0x7,0xff,0x18,0x6e,0x8e,0xdd,0x87,0xff,0x80,0x30,\
0xe0,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x30,\
0xf0,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x80,0x30,\
0xf0,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x30,\
0xf0,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x30,\
0xf0,0x7,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x70,\
0xf0,0x3,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x0,0x70,\
0xf0,0x3,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x0,0x70,\
0xf8,0x3,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x0,0x70,\
0xf8,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x0,0xf0,\
0xf8,0x1,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x0,0xf0,\
0xfc,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x0,0xf0,\
0xfc,0x0,0xff,0xff,0xff,0xff,0xff,0xff,0xfc,0x1,0xf0,\
0xfc,0x3,0xff,0xff,0xff,0xff,0xff,0xff,0xfe,0x1,0xf0,\
0xfe,0xf,0xff,0xff,0xff,0xff,0xff,0xff,0xf7,0x81,0xf0,\
0xfe,0x1e,0x3f,0xff,0xff,0xff,0xff,0xff,0xe3,0xe3,0xf0,\
0xff,0x78,0x1f,0xff,0xff,0xff,0xff,0xff,0xe0,0xfb,0xf0,\
0xff,0xf0,0xf,0xff,0xff,0xff,0xff,0xff,0xc0,0x3f,0xf0,\
0xff,0xc0,0xf,0xff,0xff,0xff,0xff,0xff,0x80,0x1f,0xf0,\
0xff,0x80,0x7,0xff,0xff,0xff,0xff,0xff,0x0,0xf,0xf0,\
0xff,0xc0,0x3,0xff,0xff,0xff,0xff,0xfe,0x0,0x1f,0xf0,\
0xff,0xe0,0x0,0xff,0xff,0xff,0xff,0xfc,0x0,0x1f,0xf0,\
0xff,0xf0,0x0,0x7f,0xff,0xff,0xff,0xf8,0x0,0x3f,0xf0,\
0xff,0xf0,0x0,0x3f,0xff,0xff,0xff,0xe0,0x0,0x7f,0xf0,\
0xff,0xf8,0x0,0x3f,0xff,0xff,0xff,0xe0,0x0,0xff,0xf0,\
0xff,0xfc,0x0,0x33,0xff,0xff,0xff,0x70,0x1,0xff,0xf0,\
0xff,0xfe,0x0,0x70,0xff,0xff,0xf8,0x30,0x3,0xff,0xf0,\
0xff,0xff,0x0,0x60,0x1f,0xff,0xc0,0x38,0x7,0xff,0xf0,\
0xff,0xff,0x80,0xe0,0x0,0x70,0x0,0x18,0xf,0xff,0xf0,\
0xff,0xff,0xe1,0xc0,0x0,0x30,0x0,0x1c,0x1f,0xff,0xf0,\
0xff,0xff,0xf1,0x80,0x0,0x30,0x0,0xe,0x7f,0xff,0xf0,\
0xff,0xff,0xff,0x80,0x0,0x30,0x0,0x6,0xff,0xff,0xf0,\
0xff,0xff,0xff,0x0,0x0,0x30,0x0,0x7,0xff,0xff,0xf0,\
0xff,0xff,0xff,0x80,0x0,0x30,0x0,0xf,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xe0,0x0,0x30,0x0,0x3f,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xfc,0x0,0x30,0x0,0xff,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xff,0x80,0x30,0xf,0xff,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xff,0xfe,0x73,0xff,0xff,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,\
0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xf0,\
}
#define LOADING_W 84
#define LOADING_H 83

RTC_DATA_ATTR int page = 1;
RTC_DATA_ATTR int lastpage = 99;

SdFile file;
Inkplate display(INKPLATE_1BIT);

char url[90];

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
    display.printf("%d", (page < lastpage)?(page + 1):lastpage);         
}

void showPage(int pagenum, uint8_t do_annotate = 1) {
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
  if(do_annotate) {
      annotate();
  }
  display.display();
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
    {
      uint8_t loading[] = LOADING;
      display.drawImage(loading, random(500), random(500), LOADING_W, LOADING_H, WHITE, BLACK);
    }
    display.partialUpdate(true);
 
  if (display.rtcCheckAlarmFlag()) {
    display.rtcClearAlarmFlag();
    drawxkcd(NEWESTXKCD); // Check day of week to see if it is an xkcd day?
    gotosleep();
  }

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
    if ((padStatus & 4) && page < lastpage) { // pad3
          page++;
    }

    if (!(padStatus & 2)) {
      showPage(page);
      gotosleep();
    }

      // Pad2
      page=1;
      wifiup();
      
      getPages();
       
      gotosleep();

}


/* download available pages from the virtual printer
 * display the first page right away then continue fetching
 * pages and saving them to SDcard
 */
void getPages() {
  int32_t imglen = 0;
  uint8_t* imgbuffer;
  imgbuffer = (uint8_t*)ps_malloc(IMGBUFSIZE * sizeof(uint8_t));
  if (imgbuffer == NULL) {
    display.println("Malloc for page buffer failed");
    display.display();
    return;
  }
  lastpage=1;
   // file.rmRfStar(); DOES NOT WORK
    do {
        sprintf(url, "%s/page%d.png", PAGEBASEURL, lastpage);
        imglen = loadhttp(url, imgbuffer, IMGBUFSIZE);
        
        if (imglen != 0) {
          if (lastpage == 1) {
              display.selectDisplayMode(INKPLATE_3BIT);
              display.drawPngFromBuffer(imgbuffer, 0, 0, imglen,false, false);
          }
          if (!file.open(strrchr(url,'/'), O_WRITE|O_CREAT)) {
            display.println("SDCard file open error");
            display.println(strrchr(url, '/'));
            display.display();
            imglen = 0;
          } else {
            file.write(imgbuffer, imglen);
            file.flush();
            file.close();
          }
         
          lastpage += 1;
        }
    } while(imglen > 0);
    lastpage -= 1;
    free(imgbuffer);
    annotate();
    display.display();
}
/** Replace all instances of an html code with a single character in the passed string
 */
void htmlunescape(char* string, const char* htmlcode, const char replacement) {

        char* strptr = strstr(string, htmlcode );
          // un-escape apostrophtes (modifies alt-text buffer)
        uint8_t code_len = strlen(htmlcode);
        while (strptr != NULL) {
          strptr[0] = replacement;
          memmove(&strptr[1], &strptr[code_len], strlen(&strptr[code_len]) + 1);
          strptr = strstr(strptr, htmlcode );
        }
}
/* load a great comic that happens to look good on an epaper display 
 *  mode random/newest
 */
void drawxkcd(uint8_t mode) {
    char *strmatch;
    char *strend;
    char *strptr;
    uint8_t* imgbuffer;
    const char imganchor[]="<img id=\"comic\" src=\"";
    const char altanchor[]="<p id=\"altText\">";
    const char titleanchor[]="alt=\"";
    char xkcdurl[80] = "https://m.xkcd.com/";// reused later for image url";
    imgbuffer = (uint8_t*)malloc(HTMLBUFSIZE * sizeof(uint8_t));
    if (imgbuffer == NULL) {
      display.println("Malloc for html buffer failed");
      display.partialUpdate();
    return;
  }
    wifiup();

    if (mode == RANDOMXKCD) {
      char rnd[8];
      sprintf(rnd,"%d/", random(2500));
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
    
    display.drawImage(xkcdurl, Image::Format::PNG, Image::Position::Center, false, false);
    strmatch = strstr(strmatch, titleanchor);
    if (strmatch != NULL) {
      strmatch += strlen(titleanchor);
      strend = strchr(strmatch,'"');
      display.setCursor(10,40);
      display.printf("%.*s\n", (strend - strmatch), strmatch); 
      
    }
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
        else
        {
          *strend = '\0';
          display.setTextSize(3);

          // un-escape html chars (modifies alt-text buffer)
          htmlunescape(strmatch, "&#39;", '\'');
          htmlunescape(strmatch, "&amp;", '&');
          htmlunescape(strmatch, "&quot;", '"');

          display.print(strmatch);
        }
    }
    free(imgbuffer);
    annotate();
    display.display();
}

/* Set things for wakeup via RTC alarm or touchpad.  goto is a good name for this as 
 *  it does not return.
 */
void gotosleep() {
    WiFi.mode(WIFI_OFF);
    if (display.rtcIsSet()) {
        display.rtcSetAlarm(00 /*sec*/, 00 /*min*/, 3 /*hour*/, 99 /*day*/, 99 /*weekday*/);
    }   
    // Setup mcp interrupts
    for (int touchPadPin = 10; touchPadPin <=12; touchPadPin++) {
      display.pinModeInternal(MCP23017_INT_ADDR, display.mcpRegsInt, touchPadPin, INPUT);
      display.setIntOutputInternal(MCP23017_INT_ADDR, display.mcpRegsInt, 1, false, false, HIGH);
      display.setIntPinInternal(MCP23017_INT_ADDR, display.mcpRegsInt, touchPadPin, RISING);
    }

    // Both wakup mechanisms are used here because touchpads pull up on wake while alarm
    // pulls down on wake
    esp_sleep_enable_ext1_wakeup((1ULL << 39), ESP_EXT1_WAKEUP_ALL_LOW); 
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
