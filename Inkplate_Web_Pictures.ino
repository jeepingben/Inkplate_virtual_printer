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
uint8_t imgbuffer[102400];
char url[45]; // "http://jeepingben.net/epaper-bmps/pagexx.png"

byte touchPadPin = 10;
byte padStatus = 0;
void annotate() {
    display.setCursor(480, 790); // Set new print position (right above first touchpad)
    display.print('-');          // Print minus sign
    display.setCursor(580, 790); // Set new print position (right above second touchpad)
    display.print('0');          // Print zero
    display.setCursor(680, 790); // Set new print position (right above third touchpad)
    display.print('+');          // Print plus sign
}
void setup()
{
    int32_t imglen;
    if (0) { //reason == timer
      drawxkcd();
      gotosleep();
    } else {}    //other
    
    if (!display.sdCardInit()) {
       display.println("Failed to initialize SD card");
       drawxkcd();
       gotosleep();
    }

    padStatus &= display.readTouchpad(PAD1);
    padStatus &= (display.readTouchpad(PAD2) << 1);
    padStatus &= (display.readTouchpad(PAD3) << 2);

    if (padStatus & 1) { //pad1
      if (page > 1) {
          page--;
          showPage(page);
          gotosleep();
      }
    }
    if (padStatus & 4) { // pad3
        if (page < 99) {
          page++;
          showPage(page);
          gotosleep();
        }
    }

    // Pad2 or power connected, etc
      page=1;
      wifiup();
      display.selectDisplayMode(INKPLATE_3BIT);

      do {
          sprintf(url, "https://jeepingben.net/epaper-bmps/page%d.png", page);
          imglen = loadhttp(url);
          if (page == 1) {
              display.drawImage(imgbuffer, 0, 0, false, false);
              annotate();
              display.display();
          }
          file.rmRfStar();
          if (!file.open(&url[33], O_WRITE)) {
            display.println("SDCard file open error");
            display.display();
          } else {
            file.write(imgbuffer, imglen);
          }
              
             
      } while(imglen > 0);
      
       WiFi.mode(WIFI_OFF);
      gotosleep();



    display.selectDisplayMode(INKPLATE_3BIT);

    if (!display.drawImage("https://jeepingben.net/epaper-bmps/page1.png", 0, 0, false, false))
    {
        // If is something failed (wrong filename or format), write error message on the screen.
        display.println("Image open error");
        display.display();
    }
    display.display();


}

void showPage(int pagenum) {
  char* filename="pagexx.png";
  filename[4]= '0' + (page / 10); // becomes unreadable for page > 99
  filename[5]= '0' + (page % 10);
  if (!display.drawImage(filename, 0, 0, false, false))
  {
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

int32_t loadhttp(char* url) {
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
          return http.getStreamPtr()->readBytes(imgbuffer, ((len > sizeof(imgbuffer)) ? sizeof(imgbuffer) : len));
        }
    }
    return NULL;
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
