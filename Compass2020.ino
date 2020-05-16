#include <Adafruit_SSD1306.h>
#include <QMC5883LCompass.h>
#include <math.h>
#include <EEPROM.h>
//#include <nRF24L01.h>
#include <RF24.h>
//#include <RF24_config.h>
#include <SPI.h>

#define WIDTH 128
#define HEIGHT 64
#define OLED_RESET 4

// -----------------------------------------------------------------
// BLUETOOTH
const int pinCE = 9;
const int pinCSN = 10;
RF24 radio(pinCE, pinCSN);

// Single radio pipe address for the 2 nodes to communicate.
const uint64_t pipe = 0xE8E8F0F0E1LL;
long data;
// -----------------------------------------------------------------

QMC5883LCompass compass;
Adafruit_SSD1306 oled(WIDTH,HEIGHT,&Wire,OLED_RESET);
const int TEXTW_PIXELS=6;
const int TEXTH_PIXELS=8;
const int TEXT_SIZE=1;
const int HALF_W=WIDTH/2;
const int HALF_H=HEIGHT/2;
//Pin to know if the user wants a new reference point;
int reference_enable = 6;
int azimuth, ref=0;
float pixelW,pixelH,x,y;

void display(int degree);
void cardinal(void);
void deg(int orientation);
void reference_set(void);

void setup() {
  pinMode(reference_enable,INPUT);
  Serial.begin(9600);
  radio.begin();
  radio.openWritingPipe(pipe);
  Wire.begin();
  oled.begin(SSD1306_SWITCHCAPVCC,0x3c);
  oled.setTextColor(WHITE);
  oled.dim(false);
  compass.init();
  if(digitalRead(reference_enable)==LOW)
  {
    reference_set();
  }
  else{
    ref=EEPROM.read(0);
  }
}

void loop() {
  // Read compass values
  compass.read();
  // Return Azimuth reading
  azimuth = compass.getAzimuth()-ref;
  if(azimuth<0)
  {
      azimuth=azimuth+360;
  }
  //azimuth= azimuth < 0 ? 360 + azimuth : azimuth;
  Serial.print("A: ");
  Serial.print(azimuth);
  Serial.println();
  display(azimuth);
}

void display(int degree){
  oled.clearDisplay();
  cardinal();
  deg(degree);
  x=cos((M_PI*degree)/180);
  y=sin((M_PI*degree)/180);
  oled.drawLine(HALF_W+(((TEXTW_PIXELS*1.5)+3)*y),HALF_H-(((TEXTW_PIXELS*1.5)+3)*x),HALF_W+((HALF_H)*y),HALF_H-((HALF_H)*x),WHITE);
  oled.drawCircle(HALF_W, HALF_H, HALF_H-10, WHITE);
  oled.drawCircle(HALF_W, HALF_H, (TEXTW_PIXELS*1.5)+5, WHITE);
  oled.display();
  delay(200);
}

void cardinal(void){
    oled.setTextSize(TEXT_SIZE);
    oled.setCursor(HALF_W-(TEXTW_PIXELS/2),0);
    oled.print("N");
    oled.setCursor(HALF_W-(TEXTW_PIXELS/2),HEIGHT-TEXTH_PIXELS+1);
    oled.print("S");
    oled.setCursor(HALF_W-HALF_H+8-TEXTW_PIXELS,HALF_H-(TEXTH_PIXELS/2));
    oled.print("W");
    oled.setCursor(HALF_W+HALF_H-12+TEXTW_PIXELS,HALF_H-(TEXTH_PIXELS/2));
    oled.print("E");
}

void deg(int orientation){
  pixelH = 0.5*TEXTH_PIXELS;
  if (orientation>=100)
  {
      pixelW = 0.5*3*TEXTW_PIXELS;   
  }
  else if (orientation>=10)
  {
      pixelW = TEXTW_PIXELS;
  }
  else
  {
      pixelW = 0.5*TEXTW_PIXELS;
  }
  oled.setCursor(HALF_W-pixelW,HALF_H-pixelH);
  oled.setTextSize(TEXT_SIZE);
  oled.print(orientation);
}

void reference_set(void)
{
    oled.setTextSize(3);
    oled.setTextColor(WHITE);
    for(int iter=3;iter>0;iter--)
    {
      oled.clearDisplay();
      oled.setCursor(HALF_W-(0.5*3*pixelW),HALF_H-(0.5*3*pixelH));
      oled.print(iter);
      oled.display();
      delay(1000);
    }
    compass.read();
    ref = compass.getAzimuth();
   //EEPROM.write(0,(int)ref);
    radio.write(&ref, sizeof ref);
    delay(1000);
    Serial.println("La referencia se definio en:");
    Serial.println(ref);
}
