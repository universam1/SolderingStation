//*******************************//
// Soldering Station
// Matthias Wagner
// www.k-pank.de/so
// Get.A.Soldering.Station@gmail.com
//*******************************//

#define __PROG_TYPES_COMPAT__ 
#include <FastLED.h>      
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include "iron.h"
#include "stationLOGO.h"

#define VERSION "1.5"		//Version der Steuerung
#define INTRO

#define LED_PIN 6
#define BRIGHTNESS 200

//Missing color in TFT lib
#define ST7735_GREY 0x632C

#define STANDBYin A4
#define POTI   	  A5
#define TEMPin  	A7
#define PWMpin 	  3
#define BLpin		  7
#define BLpinold  5

#define CNTRL_GAIN 10

#define DELAY_MAIN_LOOP 	  10
#define DELAY_MEASURE 		  50
#define ADC_TO_TEMP_GAIN 	   0.53 //Mit original Weller Station verglichen
#define ADC_TO_TEMP_OFFSET  25.0
#define STANDBY_TEMP			 175

#define OVER_SHOT 			     2
#define MAX_PWM_LOW			   180
#define MAX_PWM_HI			   255		//254
#define MAX_POTI				   400		//400Grad C

#define JITTER                    2  // Anzeigehysterese um die Anzeige zu beruhigen
#define DISPLAY_REFRESH        2000  // Zeit um die Hysterese zu initialiseren
#define DISPLAY_WRITE           200  // Schreibzyklus auf das Display
#define DISPLAY_FADE_DIFF_TEMP   40  // Temperaturabstand für den Übergang von rot nach grün = gelb
#define DISPLAY_FADE_DIFF        35  // Temperaturband für das Aus/Einblenden von rot bzw. grün
                                     // DISPLAY_FADE_DIFF muss kleiner sein als DISPLAY_FADE_DIFF_TEMP

#define PWM_DIV 1024						//default: 64   31250/64 = 2ms

//V1.5
#define dc       9   
#define rst     12     
#define cs_tft  10 

Adafruit_ST7735 tft = Adafruit_ST7735(cs_tft, dc, rst);  // Invoke custom library

int pwm = 0; //pwm Out Val 0.. 255
int soll_temp = 300;
int disp_soll_temp = 0;
int disp_ist_temp = 0;
int disp_soll_temp_old = 0;
int disp_ist_temp_old = 0;
boolean standby_act = false;
unsigned long refreshMillis = 0;
unsigned long writeMillis = 0;
uint8_t standby_act_old = false;
CRGB led[1];

void displaySB(uint8_t SBon);
uint16_t Color565(uint8_t r, uint8_t g, uint8_t b);

void setup(void) {

  // Serial.begin(9600);

	FastLED.addLeds<WS2812B, LED_PIN, GRB>(led, 1);
	FastLED.setBrightness(BRIGHTNESS);
	led[0] = CRGB::White;
	
	pinMode(BLpin, OUTPUT);     // pin 7 geschaltet über Transistor
  pinMode(BLpinold, INPUT);   // pin 5 ist direkt mit BL verbunden und wird als Eingang definiert um nicht zu stören
	digitalWrite(BLpin, LOW);
	
	pinMode(STANDBYin, INPUT_PULLUP);
	
	pinMode(PWMpin, OUTPUT);
	digitalWrite(PWMpin, LOW);
	setPwmFrequency(PWMpin, PWM_DIV);
	digitalWrite(PWMpin, LOW);
	
	tft.initR(INITR_BLACKTAB);
	SPI.setClockDivider(SPI_CLOCK_DIV4);  // 4MHz
	
	tft.setRotation(0);	// 0 - Portrait, 1 - Lanscape
	tft.fillScreen(ST7735_BLACK);
	tft.setTextWrap(false);
	
	//Print station Logo
	tft.drawBitmap(2,1,stationLOGO1,124,47,ST7735_GREY);
	tft.drawBitmap(3,3,stationLOGO1,124,47,ST7735_YELLOW);		
	tft.drawBitmap(3,3,stationLOGO2,124,47,Color565(254,147,52));	
	tft.drawBitmap(3,3,stationLOGO3,124,47,Color565(255,78,0));
	
	//BAcklight on
	digitalWrite(BLpin, HIGH);
	
#if defined(INTRO)
	delay(500);
	
	//Print Iron
	tft.drawBitmap(15,50,iron,100,106,ST7735_GREY);
	tft.drawBitmap(17,52,iron,100,106,ST7735_YELLOW);
	delay(500);
	
	tft.setTextSize(2);
	tft.setTextColor(ST7735_GREY);
	tft.setCursor(70,130);
	tft.print(VERSION);
	
	tft.setTextSize(2);
	tft.setTextColor(ST7735_YELLOW);
	tft.setCursor(72,132);
	tft.print(VERSION);
	
	tft.setTextSize(1);
	tft.setTextColor(ST7735_GREY);
	tft.setCursor(102,0);
	tft.print("v");
	tft.print(VERSION);
	
	tft.setTextColor(ST7735_YELLOW);
	tft.setCursor(103,1);
	tft.print("v");
	tft.print(VERSION);
	
	delay(2500);
#endif
	
	tft.fillRect(0,47,128,125,ST7735_BLACK);
	tft.setTextColor(ST7735_WHITE);

	tft.setTextSize(1);
	tft.setCursor(1,85);
	tft.print("ist");
	
	tft.setTextSize(2);
	tft.setCursor(117,49);
	tft.print("o");
	
	tft.setTextSize(1);
	tft.setCursor(1,130);
	tft.print("soll");
	
	tft.setTextSize(2);
	tft.setCursor(117,94);
	tft.print("o");
	
	tft.setCursor(80,144);
	tft.print("   %");
	
	tft.setTextSize(1);
	tft.setCursor(1,151);		//60
	tft.print("pwm");
	
	tft.setTextSize(2);
}


void loop() {
	
	int actual_temperature = getTemperature();
  unsigned long currentMillis = millis();

  soll_temp = map(analogRead(POTI), 0, 1024, 0, MAX_POTI);

  if ((actual_temperature-JITTER > disp_ist_temp) || (actual_temperature+JITTER < disp_ist_temp)){
    disp_ist_temp = actual_temperature;
  }
  
  if ((soll_temp-JITTER > disp_soll_temp) || (soll_temp+JITTER < disp_soll_temp)){
    disp_soll_temp = soll_temp;
  }
  
  if ((currentMillis - refreshMillis >= DISPLAY_REFRESH) || (abs(soll_temp-actual_temperature) > 10) ) {
    refreshMillis = currentMillis;
    disp_soll_temp = soll_temp;
    disp_ist_temp = actual_temperature;
  }

	int soll_temp_tmp = soll_temp;
	
	if (digitalRead(STANDBYin) == false) {
		standby_act = true;
	} else {
		standby_act = false;
	}
 
  if (standby_act != standby_act_old) displaySB(standby_act);
	
	if (standby_act && (soll_temp >= STANDBY_TEMP )) {
		soll_temp_tmp = STANDBY_TEMP;
	}
	
	int diff = (soll_temp_tmp + OVER_SHOT)- actual_temperature;
	pwm = diff*CNTRL_GAIN;
	
	int MAX_PWM;

	//Set max heating Power 
	MAX_PWM = standby_act ? MAX_PWM_LOW : MAX_PWM_HI;
	
	//8 Bit Range
	pwm = pwm > MAX_PWM ? pwm = MAX_PWM : pwm < 0 ? pwm = 0 : pwm;
	
	//NOTfall sicherheit / Spitze nicht eingesteckt
	if (actual_temperature > 550){
		pwm = 0;
		actual_temperature = 0;
	}
	
	analogWrite(PWMpin, pwm);
	//digitalWrite(PWMpin, LOW);
	
	if (currentMillis - writeMillis >= DISPLAY_WRITE) {
	  writeMillis = currentMillis;
	  writeHEATING(disp_soll_temp, disp_ist_temp, pwm);
	}
 
	//update LED
	FastLED.show();
 
	disp_soll_temp_old = disp_soll_temp;
  disp_ist_temp_old = disp_ist_temp;
  standby_act_old = standby_act;

	delay(DELAY_MAIN_LOOP);		//wait for some time
}


int getTemperature()
{
	analogWrite(PWMpin, 0);		//switch off heater
	delay(DELAY_MEASURE);			//wait for some time (to get low pass filter in steady state)
	int adcValue = analogRead(TEMPin); // read the input on analog pin 7:
	//Serial.print("ADC Value ");
	//Serial.print(adcValue);
	analogWrite(PWMpin, pwm);	//switch heater back to last value
	return round(((float) adcValue)*ADC_TO_TEMP_GAIN+ADC_TO_TEMP_OFFSET); //apply linear conversion to actual temperature
}

void writeHEATING(int tempSOLL, int tempVAL, int pwmVAL){

	static int tempSOLL_OLD = 	10;
	static int tempVAL_OLD	= 	10;
	static int pwmVAL_OLD	= 	10;

	//TFT Anzeige
	
	pwmVAL = map(pwmVAL, 0, 254, 0, 100);
	
	tft.setTextSize(5);
	if (tempVAL_OLD != tempVAL){
		tft.setCursor(30,57);
		tft.setTextColor(ST7735_BLACK);
		if ((tempVAL_OLD/100) != (tempVAL/100)){
			tft.print(tempVAL_OLD/100);
		} else {
			tft.print(" ");
		}
		if ( ((tempVAL_OLD/10)%10) != ((tempVAL/10)%10) ) {
			tft.print((tempVAL_OLD/10)%10 );
		} else {
			tft.print(" ");
		}
		if ( (tempVAL_OLD%10) != (tempVAL%10) ) {
			tft.print(tempVAL_OLD%10 );
		}
		tft.setCursor(30,57);
		tft.setTextColor(ST7735_WHITE);
		if (tempVAL < 100) {
			tft.print(" ");
		}
		if (tempVAL <10) {
			tft.print(" ");
		}
    if (standby_act) {
      tft.setTextColor(ST7735_CYAN);
      led[0].r = 0;
      led[0].g = 0;
      led[0].b = 255;
    } else {
      int tempDIV = tempSOLL - tempVAL;
      tempDIV = tempDIV > 255 ? tempDIV = 255 : tempDIV < 0 ? tempDIV = 0 : tempDIV;
      int tempR = (tempDIV*(255/DISPLAY_FADE_DIFF))-(DISPLAY_FADE_DIFF_TEMP-DISPLAY_FADE_DIFF);
      tempR = tempDIV >= DISPLAY_FADE_DIFF_TEMP ? tempR = 255 : tempR > 255 ? tempR = 255 : tempR < 0 ? tempR = 0 : tempR;
      int tempG = 255+(DISPLAY_FADE_DIFF_TEMP*(255/DISPLAY_FADE_DIFF))-(tempDIV*(255/DISPLAY_FADE_DIFF));
      tempG = tempDIV <= DISPLAY_FADE_DIFF_TEMP ? tempG = 255 : tempG < 0 ? tempG = 0 : tempG > 255 ? tempG = 255 : tempG;
      tft.setTextColor(Color565(tempR, tempG, 0));
      led[0].r = tempR;
      led[0].g = tempG;
      led[0].b = 0;
      tft.print(tempVAL);
    }
		tempVAL_OLD = tempVAL;
	}
	
	if (tempSOLL_OLD != tempSOLL){
		tft.setCursor(30,102);
		tft.setTextColor(ST7735_BLACK);
		if ((tempSOLL_OLD/100) != (tempSOLL/100)){
			tft.print(tempSOLL_OLD/100);
		} else {
			tft.print(" ");
		}
		if ( ((tempSOLL_OLD/10)%10) != ((tempSOLL/10)%10) ) {
			tft.print((tempSOLL_OLD/10)%10 );
		} else {
			tft.print(" ");
		}
		if ( (tempSOLL_OLD%10) != (tempSOLL%10) ) {
			tft.print(tempSOLL_OLD%10 );
		}
		
		//Neuen Wert in Weiß schreiben
		tft.setCursor(30,102);
		tft.setTextColor(ST7735_WHITE);
		if (tempSOLL < 100) {
			tft.print(" ");
		}
		if (tempSOLL <10) {
			tft.print(" ");
		}
		tft.print(tempSOLL);
		tempSOLL_OLD = tempSOLL;
	}
	
	
	tft.setTextSize(2);
	if (pwmVAL_OLD != pwmVAL){
		tft.setCursor(80,144);
		tft.setTextColor(ST7735_BLACK);
		if ((pwmVAL_OLD/100) != (pwmVAL/100)){
			tft.print(pwmVAL_OLD/100);
		} else {
			tft.print(" ");
		}
		if ( ((pwmVAL_OLD/10)%10) != ((pwmVAL/10)%10) ) {
			tft.print((pwmVAL_OLD/10)%10 );
		} else {
			tft.print(" ");
		}
		if ( (pwmVAL_OLD%10) != (pwmVAL%10) ) {
			tft.print(pwmVAL_OLD%10 );
		}
		tft.setCursor(80,144);
		tft.setTextColor(ST7735_WHITE);
		if (pwmVAL < 100) {
			tft.print(" ");
		}
		if (pwmVAL <10) {
			tft.print(" ");
		}
		tft.print(pwmVAL);
		pwmVAL_OLD = pwmVAL;
	}
}

void setPwmFrequency(int pin, int divisor) {
	byte mode;
	if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
		switch(divisor) {
			case 1: 		mode = 0x01; break;
			case 8: 		mode = 0x02; break;
			case 64: 		mode = 0x03; break;
			case 256: 	mode = 0x04; break;
			case 1024: 	mode = 0x05; break;
			default: return;
		}
		
		if(pin == 5 || pin == 6) {
			TCCR0B = TCCR0B & 0b11111000 | mode;
		} else {
			TCCR1B = TCCR1B & 0b11111000 | mode;
		}
	} else if(pin == 3 || pin == 11) {
		switch(divisor) {
			case 1: 		mode = 0x01; break;
			case 8: 		mode = 0x02; break;
			case 32: 		mode = 0x03; break;
			case 64: 		mode = 0x04; break;
			case 128: 	mode = 0x05; break;
			case 256: 	mode = 0x06; break;
			case 1024: 	mode = 0x07; break;
			default: return;
		}
		
		TCCR2B = TCCR2B & 0b11111000 | mode;
	}
}

void displaySB(uint8_t SBon) {
  tft.setCursor(2,55);
  if (SBon) {
    tft.setTextColor(ST7735_WHITE);
  } else {
    tft.setTextColor(ST7735_BLACK);
  }
  tft.print("SB");
}

uint16_t Color565(uint8_t r, uint8_t g, uint8_t b) {
 return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

