/*
  Download and display pages from the virtual printer on jeepingben.net
  load xkcd overnight
*/

#include "HTTPClient.h"
#include "Inkplate.h"
#include "WiFi.h"
#include "HTTPClient.h"
RTC_DATA_ATTR int page = 1;

Inkplate display(INKPLATE_1BIT);

const char ssid[] = "Maine Volcano Observatory";    // Your WiFi SSID
const char *password = "Eufm-Qmp2-rzrp-AgaL"; // Your WiFi password

byte touchPadPin = 10;
byte padStatus = 0;
void setup()
{
    char[45] url; // "http://jeepingben.net/epaper-bmps/pagexx.png");
    char* imgbuffer;

    reason == timer
      drawxkcd();
      gotosleep();


    other
    if (!Inkplate::sdCardInit()) {
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
          showpage(page);
          gotosleep();
      }
    }
    if (padStatus & 4) { // pad3
        if (page < 99) {
          page++;
          showpage(page);
          gotosleep();
        }
    }

    // Pad2 or power connected, etc
      page=1;
      wifiup();
      HTTPClient http;
      // Set parameters to speed up the download process.
      http.getStream().setNoDelay(true);
      http.getStream().setTimeout(1);
      sprintf(url, "https://jeepingben.net/epaper-bmps/page%d.png", page);
      imgbuffer = loadhttp(url);
      if (imgbuffer != null) {
        display.selectDisplayMode(INKPLATE_3BIT);
        display
        save
      download other pages
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
    wifiup()
    // draw xkcd image
    // get alttext
    // write alttext
    annotate();
    display.display();
    WiFi.mode(WIFI_OFF);
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
char* loadhttp(char* url) {
  http.begin(url);
  if (httpCode == 200)
    {
        // Get the response length and make sure it is not 0.
        int32_t len = http.getSize();
        if (len > 0) {

          // BRD - need to buffer this stream + return char* - stream != char*!
          //   // read up to 128k byte
          char[128*1024] buff;
int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          return http.getStreamPtr();
        }
    }
    return null;
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
