#include <Key.h>
#include <Keypad.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include <TftSpfd5408.h>      // Hardware-specific library
#include <DHT.h>              // DHT22 sensor library

#define LCD_CS      A3        // Chip Select goes to Analog 3
#define LCD_CD      A2        // Command/Data goes to Analog 2
#define LCD_WR      A1        // LCD Write goes to Analog 1
#define LCD_RD      A0        // LCD Read goes to Analog 0
#define LCD_RESET   A4        // Can alternately just connect to Arduino's reset pin
#define MPX_IN      A8        // voltage input from MPX4115A barometric sensor (P)
#define DHT_PIN     30        // pin for 1wire communication with DHT22 sensor (T, H)
#define DHT_DELAY   2500      // delay between two measurements

#define DHT_TYPE    DHT11     // ---------------------------
#define	BLACK       0x0000    
#define	BLUE        0x001F
#define	RED         0xF800
#define	GREEN       0x07E0
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define LCD_ROTATION 1

#define LED         13

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = 
{
  {'1','2','3','A'},
  {'4','5','6','B'},  
  {'7','8','9','C'},
  {'S','0','T','D'}
};

byte colPins[] = {37,36,35,34};
byte rowPins[] = {41,40,39,38};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// function prototypes
void printXY(int X, int Y, int tSize, int fColor, int bColor, char* text);
void valueXY(int X, int Y, int tSize, int fColor, int bColor, float value);
void checkKey(void);

// constant defs
const int buttonPins[] = {49,50,51,52,53};
const int ledPin = 13;

// object references
TftSpfd5408 tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
DHT dht(DHT_PIN, DHT_TYPE);

// variables
float pressure = 0.0;
float hif = 0.0;
float hic = 0.0;
int buttonState = 0;
int changed = 0;
char key[1] = "";

void keypadEvent(KeypadEvent eKey)
{
  switch (keypad.getState())
  {
    case PRESSED:
    {
      digitalWrite(LED, HIGH);
      delay(50);
      digitalWrite(LED, LOW);
      delay(50);
      sprintf(key,"%c",eKey);
      printXY(10,125,2,GREEN,BLACK,key);
      delay(300);
      printXY(10,125,2,BLACK,BLACK,key);
      delay(100);
      Serial.print(key);
      key[1] = "";
    } //END case
  } //END switch 
} //END keypadEvent

void setup(void) {
  pinMode(MPX_IN, INPUT);
  pinMode(DHT_PIN, INPUT);

  keypad.addEventListener(keypadEvent);

  Serial.begin(9600);
  Serial.println("2017 (c) HAMTRONIX s.r.o.");
  Serial.println("-------------------------------------------------");
  Serial.println("Firmware: 0.11a");
  Serial.println("READY..");
  Serial.println("-------------------------------------------------");
  
  pinMode(50, INPUT_PULLUP);
  pinMode(51, INPUT_PULLUP);
  pinMode(52, INPUT_PULLUP);
  pinMode(53, INPUT_PULLUP);
  
  pinMode(ledPin, OUTPUT);
  
  tft.reset();
  tft.begin(0x9341);
  delay(100);
  tft.setRotation(LCD_ROTATION);
  tft.fillScreen(BLACK);
  
  dht.begin();

  printXY(10,25,2,RED,BLACK,"Temperature: "); 
  printXY(10,45,2,YELLOW,BLACK,"Humidity: ");
  printXY(10,65,2,CYAN,BLACK,"Humidity ind: "); 
  printXY(10,85,2,GREEN,BLACK,"Pressure: ");
}

void loop(void) {
  unsigned long previousMillis = 0;
  float h = 0.0;
  float t = 0.0;
  float f = 0.0;
  
  unsigned long currentMillis = millis();                           // zaciatok odpoctu
    
  keypad.getKey();
  // read value from MPX4115A
  pressure = (analogRead(MPX_IN)*5.0)/1024;
  pressure = ((pressure + 0.475)/0.00475) + 90;       // conversion to hPa (millibars)
                                                      // altitude compensation for 330m over see level
  // read T and H from DHT11
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)

  if ( currentMillis - previousMillis >= DHT_DELAY ) {
    previousMillis = currentMillis;
    h = dht.readHumidity();
    t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    f = dht.readTemperature(true);
    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) 
    {
      printXY(10, 205, 2, YELLOW, RED, "Read from DHT failed !");
      h = 0.0;
      t = 0.0;
      f = 0.0;
    }
    else {
      printXY(10, 205, 2, BLACK, BLACK, "Read from DHT failed !");
      // Compute heat index in Fahrenheit (the default)
      hif = dht.computeHeatIndex(f, h);
      // Compute heat index in Celsius (isFahreheit = false)
      hic = dht.computeHeatIndex(t, h, false);
    }
    printXY(175, 25, 2, BLACK, BLACK, "     ");
    valueXY(175, 25, 2, RED, BLACK, t);
    printXY(260, 25, 2, RED, BLACK, " C");
  
    printXY(175, 45, 2, BLACK, BLACK, "     ");
    valueXY(175, 45, 2, YELLOW, BLACK, h);
    printXY(260, 45, 2, YELLOW, BLACK, " %");
  
    printXY(175, 65, 2, BLACK, BLACK, "     ");
    valueXY(175, 65, 2, CYAN, BLACK, hic);
    printXY(260, 65, 2, CYAN, BLACK, " %");

    printXY(175, 85, 2, BLACK, BLACK, "     ");
    valueXY(175, 85, 2, GREEN, BLACK, pressure);
    printXY(260, 85, 2, GREEN, BLACK, " hPa");

    // icons
    // gauges
  }
}

void checkKey(void) 
{
  //---
}

void printXY(int X, int Y, int tSize, int fColor, int bColor, char* text) {
  tft.setCursor(X, Y);                // cursor to position X,Y
  tft.setTextSize(tSize);             // text size to tSize
  tft.setTextColor(fColor, bColor);   // foreground and background text color
  tft.print(text);
}

void valueXY(int X, int Y, int tSize, int fColor, int bColor, float value) {
  tft.setCursor(X, Y);                // cursor to position X,Y
  tft.setTextSize(tSize);             // text size to tSize
  tft.setTextColor(fColor, bColor);   // foreground and background text color
  tft.print(value);
}
