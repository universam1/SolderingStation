//*******************************//
// Soldering Station
// Matthias Wagner
// www.k-pank.de/so
// Get.A.Soldering.Station@gmail.com
//*******************************//

#define __PROG_TYPES_COMPAT__
// #include <FastLED.h>
#include <Adafruit_GFX.h>	// Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include <SerialCommand.h>
#include "iron.h"
#include "stationLOGO.h"

#define VERSION "1.7" //Version der Steuerung
// #define INTRO

// #define LED_PIN 6
#define BRIGHTNESS 255
#define MAXWATT 40
#define INTVUPD 123

//Missing color in TFT lib
#define ST7735_GREY 0x632C

#define STANDBYin A1
#define POTI A0
#define TEMPin A2
#define PWMpin 3
#define BLpin 5
#define PINvolt A3
// #define BLpinold  5

#define CNTRL_GAIN 10
#define POTIOFF 10

#define DELAY_MAIN_LOOP 10
#define DELAY_MEASURE 10
// #define ADC_TO_TEMP_GAIN 0.53 //Mit original Weller Station verglichen
// #define ADC_TO_TEMP_OFFSET 25.0
#define ADC_TO_TEMP_GAIN 0.42
#define ADC_TO_TEMP_OFFSET 60.0
#define STANDBY_TEMP 90

#define OVER_SHOT 2
#define MAX_PWM_LOW 80
#define MAX_PWM_HI 255 //254
#define MAX_POTI 400   //400Grad C

#define JITTER 2				  // Hysterese um die Anzeige zu beruhigen
#define DISPLAY_REFRESH 500		  // Zeit um die Hysterese zu initialiseren
#define AUTO_OFF 600000			  // Zeit für das Ausschalten der Heizung (10min)
#define DISPLAY_FADE_DIFF_TEMP 20 // Temperaturabstand für den Übergang von rot nach grün = gelb
#define DISPLAY_FADE_DIFF 35	  // Temperaturband für das Aus/Einblenden von rot bzw. grün
								  // DISPLAY_FADE_DIFF muss kleiner sein als DISPLAY_FADE_DIFF_TEMP

#define PWM_DIV 1024 //default: 64   31250/64 = 2ms

// Define various ADC prescaler
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

#define RTresist 2.2
#define VOLTDIVIDER 34.0

//V1.5
#define dc 9
#define rst 12
#define cs_tft 10

Adafruit_ST7735 tft = Adafruit_ST7735(cs_tft, dc, rst); // Invoke custom library
SerialCommand sCmd;

int pwm = 0; //pwm Out Val 0.. 255
int soll_temp = 0;
int soll_temp_tmp = 0;
int disp_soll_temp = 0;
boolean standby_act = false;
boolean standby_act_old = false;
boolean autoOFF = false;
boolean autoOFF_old = false;
unsigned long refreshMillis = 0;
unsigned long autoOffMillis = 0;
unsigned long pwmMillis = 0;
// CRGB led[1];
float volt;
uint8_t watt;
uint32_t lastVupd;

uint8_t calcPower(float volt, uint16_t pwm, float resist = RTresist)
{
	float curr = volt / resist;
	curr = curr * volt / 255 * pwm;
	// Serial.print("V: ");
	// Serial.println(volt);
	// Serial.print("P: ");
	// Serial.println(curr);
	return curr;
}

float getVolt()
{
	return analogRead(PINvolt) / VOLTDIVIDER;
}

void setPower()
{
	char *arg;
	uint8_t power;
	arg = sCmd.next();
	if (arg != NULL)
	{
		power = atoi(arg);
		Serial.print("setting power: ");
		Serial.println(power);
		// not implemented
	}
	else
	{
		Serial.println("No arguments");
	}
}

uint8_t getMaxPWM(float volt, uint16_t pwm)
{
	uint16_t _maxWatt = volt * (volt / RTresist);
	_maxWatt = MAXWATT * 255 / _maxWatt;
	// Serial.println(_maxWatt);
	return constrain(_maxWatt, 0, 255);
}

void setBrightness()
{
	char *arg;
	uint16_t power;
	arg = sCmd.next();
	if (arg != NULL)
	{
		power = atoi(arg);
		Serial.print("setting brightness: ");
		Serial.println(power);
		analogWrite(BLpin, power);
	}
	else
	{
		Serial.println("No arguments");
	}
}

void setup(void)
{
	// change the prescaler for faster ADC
	// set up the ADC
	ADCSRA &= ~PS_128; // remove bits set by Arduino library
	ADCSRA |= PS_32;   // 32 prescaler

	SPI.setClockDivider(SPI_CLOCK_DIV2); // 4MHz

	Serial.begin(115200);
	sCmd.addCommand("P", setPower);
	sCmd.addCommand("B", setBrightness);

	// FastLED.addLeds<WS2812B, LED_PIN, GRB>(led, 1);
	// FastLED.setBrightness(BRIGHTNESS);
	// led[0] = CRGB::White;

	pinMode(BLpin, OUTPUT); // pin 7 geschaltet über Transistor
	// 												// pinMode(BLpinold, INPUT);   // pin 5 ist direkt mit BL verbunden und wird als Eingang definiert um nicht zu stören
	// digitalWrite(BLpin, LOW);

	pinMode(STANDBYin, INPUT_PULLUP);

	pinMode(PWMpin, OUTPUT);
	digitalWrite(PWMpin, LOW);
	setPwmFrequency(PWMpin, PWM_DIV);
	digitalWrite(PWMpin, LOW);
	analogWrite(BLpin, BRIGHTNESS);

	tft.initR(INITR_BLACKTAB);

	tft.setRotation(0); // 0 - Portrait, 1 - Lanscape
	tft.fillScreen(ST7735_BLACK);
	tft.setTextWrap(false);

	//Print station Logo
	tft.drawBitmap(2, 1, stationLOGO1, 124, 47, ST7735_GREY);
	tft.drawBitmap(3, 3, stationLOGO1, 124, 47, ST7735_YELLOW);
	tft.drawBitmap(3, 3, stationLOGO2, 124, 47, tft.Color565(254, 147, 52));
	tft.drawBitmap(3, 3, stationLOGO3, 124, 47, tft.Color565(255, 78, 0));

//BAcklight on
// digitalWrite(BLpin, HIGH);

#if defined(INTRO)
	delay(500);

	//Print Iron
	tft.drawBitmap(15, 50, iron, 100, 106, ST7735_GREY);
	tft.drawBitmap(17, 52, iron, 100, 106, ST7735_YELLOW);
	delay(500);

	tft.setTextSize(2);
	tft.setTextColor(ST7735_GREY);
	tft.setCursor(70, 130);
	tft.print(VERSION);

	tft.setTextSize(2);
	tft.setTextColor(ST7735_YELLOW);
	tft.setCursor(72, 132);
	tft.print(VERSION);

	tft.setTextSize(1);
	tft.setTextColor(ST7735_GREY);
	tft.setCursor(102, 0);
	tft.print("v");
	tft.print(VERSION);

	tft.setTextColor(ST7735_YELLOW);
	tft.setCursor(103, 1);
	tft.print("v");
	tft.print(VERSION);

	delay(2500);
#endif

	tft.fillRect(0, 47, 128, 125, ST7735_BLACK);
	tft.setTextColor(ST7735_WHITE);

	tft.setTextSize(1);
	tft.setCursor(1, 85);
	tft.print("ist");

	tft.setTextSize(2);
	tft.setCursor(117, 49);
	tft.print("o");

	tft.setTextSize(1);
	tft.setCursor(1, 128);
	tft.print("soll");

	tft.setTextSize(2);
	tft.setCursor(117, 94);
	tft.print("o");

	tft.setTextSize(1);
	tft.setCursor(1, 151); //60
	tft.print("pwm");
	// tft.setCursor(28, 144);
	// tft.print("0%");
	// tft.setCursor(104, 144);
	// // tft.print("100%");
	// tft.print();

	tft.setTextSize(2);
}

void loop()
{
	sCmd.readSerial();
	volt = getVolt();

	int actual_temperature = getTemperature();
	unsigned long currentMillis = millis();
	standby_act = !digitalRead(STANDBYin);
	int poti = analogRead(POTI);
	if (poti < POTIOFF)
		soll_temp = 0;
	else
		soll_temp = map(poti, POTIOFF, 1020, 150, MAX_POTI);

	if ((soll_temp - JITTER > disp_soll_temp) || (soll_temp + JITTER < disp_soll_temp))
	{
		disp_soll_temp = soll_temp;
	}

	if (standby_act != standby_act_old)
	{
		autoOffMillis = currentMillis;
	}

	if (standby_act && (soll_temp >= STANDBY_TEMP))
	{
		soll_temp_tmp = STANDBY_TEMP;
	}
	else
	{
		soll_temp_tmp = soll_temp;
	}

	if (currentMillis - autoOffMillis >= AUTO_OFF)
	{
		soll_temp_tmp = 0;
		autoOFF = true;
	}
	else
	{
		autoOFF = false;
	}

	int diff = (soll_temp_tmp + OVER_SHOT) - actual_temperature;
	pwm = diff * CNTRL_GAIN;

	int MAX_PWM;

	//Set max heating Power
	MAX_PWM = standby_act ? MAX_PWM_LOW : getMaxPWM(volt, pwm);

	//8 Bit Range
	pwm = pwm > MAX_PWM ? pwm = MAX_PWM : pwm < 0 ? pwm = 0 : pwm;

	//NOTfall sicherheit / Spitze nicht eingesteckt
	if (actual_temperature > 415)
	{
		pwm = 0;
		actual_temperature = 0;
	}

	analogWrite(PWMpin, pwm);
	watt = calcPower(volt, pwm);

	//digitalWrite(PWMpin, LOW);
	// calcPower(volt, pwm);

	if ((currentMillis - refreshMillis >= DISPLAY_REFRESH) ||
		((abs(soll_temp_tmp - actual_temperature) > 10) && !autoOFF && (currentMillis - refreshMillis >= DISPLAY_REFRESH / 8)) ||
		(standby_act != standby_act_old))
	{
		refreshMillis = currentMillis;
		writeHEATING(disp_soll_temp, actual_temperature);
	}

	writePWM(map(pwm, 0, MAX_PWM, 0, 100));

	//update LED
	// fastled.show();

	standby_act_old = standby_act;

	// delay(DELAY_MAIN_LOOP); //wait for some time
}

int getTemperature()
{
	analogWrite(PWMpin, 0); //switch off heater
	delay(DELAY_MEASURE);   //wait for some time (to get low pass filter in steady state)
	analogRead(TEMPin);		// read the input on analog pin 7:
	float adcValue;
	for (uint8_t i = 0; i < 8; i++)
		adcValue += analogRead(TEMPin); // read the input on analog pin 7:
	// adcValue = adcValue /4;
	// Serial.print("ADC Value ");
	// Serial.println(adcValue);
	analogWrite(PWMpin, pwm);												//switch heater back to last value
	return round(adcValue * (ADC_TO_TEMP_GAIN / 8.0) + ADC_TO_TEMP_OFFSET); //apply linear conversion to actual temperature
}

void writePWM(int pwmVAL)
{

	static int pwmVAL_OLD = 10;

	//TFT Anzeige

	// pwmVAL = map(pwmVAL, 0, 254, 0, 100);
	tft.setTextSize(2);
	if (autoOFF_old != autoOFF)
	{
		if (autoOFF)
		{
			tft.fillRect(0, 144, 128, 16, 0);
			tft.setCursor(2, 55);
			tft.setTextColor(ST7735_BLACK);
			tft.print("SB");
			tft.setCursor(0, 144);
			tft.setTextColor(ST7735_CYAN);
			tft.print("10min. OFF");
		}
		else
		{
			tft.setCursor(0, 144);
			tft.setTextColor(ST7735_BLACK);
			tft.print("10min. OFF");
			tft.setTextSize(1);
			tft.setTextColor(ST7735_WHITE);
			tft.setCursor(1, 151); //60
			tft.print("pwm");
			// tft.setCursor(28, 144);
			// tft.print("0%");
			// tft.setCursor(104, 144);
			// tft.print("100%");
			if (pwmVAL)
			{
				tft.fillRect(28, 153, pwmVAL, 12, ST7735_YELLOW);
				tft.fillRect(28 + pwmVAL, 153, 100 - pwmVAL, 12, ST7735_BLACK);
			}
			else
			{
				tft.fillRect(28, 153, 1, 12, ST7735_WHITE);
				tft.fillRect(29, 153, 99, 12, ST7735_BLACK);
			}
		}
	}
	else if ((pwmVAL_OLD != pwmVAL) && !autoOFF)
	{
		if (pwmVAL)
		{
			uint8_t _pwm = pwmVAL * 2.55;
			tft.fillRect(28, 153, pwmVAL, 12, tft.Color565(_pwm, 255 - _pwm, 0));
			if (pwmVAL != 100)
				tft.fillRect(28 + pwmVAL, 153, 100 - pwmVAL, 12, ST7735_BLACK);
		}
		else
		{
			tft.fillRect(28, 153, 1, 12, ST7735_WHITE);
			tft.fillRect(29, 153, 99, 12, ST7735_BLACK);
		}

		if (millis() - lastVupd > INTVUPD)
		{
			lastVupd = millis();

			tft.fillRect(28, 144, 30, 8, ST7735_BLACK);
			tft.setTextSize(1);
			tft.setCursor(28, 144);

			if (volt < 10.0)
			{
				tft.setTextColor(ST7735_BLACK);
				tft.print(' ');
			}
			tft.setTextColor(ST7735_WHITE);
			tft.print(volt);
			tft.print('V');

			tft.fillRect(80, 144, 30, 8, ST7735_BLACK);

			if (watt >= MAXWATT - 1)
				tft.setTextColor(ST7735_RED);
			tft.setCursor(80, 144);
			if (watt < 10)
				tft.print(' ');
			tft.print(watt);
			tft.print('W');
		}
	}
	autoOFF_old = autoOFF;
}

void writeHEATING(int tempSOLL, int tempVAL)
{

	static int tempSOLL_OLD = 10;
	static int tempVAL_OLD = 10;

	tft.setTextSize(5);
	if (tempVAL_OLD != tempVAL)
	{
		tft.setCursor(30, 57);
		tft.setTextColor(ST7735_BLACK);
		if ((tempVAL_OLD / 100) != (tempVAL / 100))
		{
			tft.print(tempVAL_OLD / 100);
		}
		else
		{
			tft.print(" ");
		}
		if (((tempVAL_OLD / 10) % 10) != ((tempVAL / 10) % 10))
		{
			tft.print((tempVAL_OLD / 10) % 10);
		}
		else
		{
			tft.print(" ");
		}
		if ((tempVAL_OLD % 10) != (tempVAL % 10))
		{
			tft.print(tempVAL_OLD % 10);
		}
		tft.setCursor(30, 57);
		tft.setTextColor(ST7735_WHITE);
		if (tempVAL < 100)
		{
			tft.print(" ");
		}
		if (tempVAL < 10)
		{
			tft.print(" ");
		}
		if (standby_act && !autoOFF)
		{
			tft.setTextColor(ST7735_BLUE);
			// led[0].r = 0;
			// led[0].g = 0;
			// led[0].b = 255;
		}
		else if (autoOFF)
		{
			tft.setTextColor(ST7735_CYAN);
			// led[0].r = 0;
			// led[0].g = 0;
			// led[0].b = 0;
		}
		else
		{
			int tempDIV = tempSOLL - tempVAL;
			tempDIV = tempDIV > 255 ? tempDIV = 255 : tempDIV < 0 ? tempDIV = 0 : tempDIV;
			int tempR = (tempDIV * (255 / DISPLAY_FADE_DIFF)) - (DISPLAY_FADE_DIFF_TEMP - DISPLAY_FADE_DIFF);
			tempR = tempDIV >= DISPLAY_FADE_DIFF_TEMP ? tempR = 255 : tempR > 255 ? tempR = 255 : tempR < 0 ? tempR = 0 : tempR;
			int tempG = 255 + (DISPLAY_FADE_DIFF_TEMP * (255 / DISPLAY_FADE_DIFF)) - (tempDIV * (255 / DISPLAY_FADE_DIFF));
			tempG = tempDIV <= DISPLAY_FADE_DIFF_TEMP ? tempG = 255 : tempG < 0 ? tempG = 0 : tempG > 255 ? tempG = 255 : tempG;
			tft.setTextColor(tft.Color565((uint8_t)tempR, (uint8_t)tempG, 0));
			// led[0].r = tempR;
			// led[0].g = tempG;
			// led[0].b = 0;
		}
		tft.print(tempVAL);
		tft.setTextSize(1);
		tft.setCursor(1, 85);
		tft.print("ist");
		tft.setTextSize(2);
		tft.setCursor(117, 49);
		tft.print("o");
		tempVAL_OLD = tempVAL;
		tft.setCursor(2, 55);
		if (standby_act && !autoOFF)
		{
			tft.setTextColor(ST7735_BLUE);
		}
		else
		{
			tft.setTextColor(ST7735_BLACK);
		}
		tft.print("SB");
	}

	tft.setTextSize(4);
	if (tempSOLL_OLD != tempSOLL)
	{
		tft.setCursor(40, 106);
		tft.setTextColor(ST7735_BLACK);
		if ((tempSOLL_OLD / 100) != (tempSOLL / 100))
		{
			tft.print(tempSOLL_OLD / 100);
		}
		else
		{
			tft.print(" ");
		}
		if (((tempSOLL_OLD / 10) % 10) != ((tempSOLL / 10) % 10))
		{
			tft.print((tempSOLL_OLD / 10) % 10);
		}
		else
		{
			tft.print(" ");
		}
		if ((tempSOLL_OLD % 10) != (tempSOLL % 10))
		{
			tft.print(tempSOLL_OLD % 10);
		}

		//Neuen Wert in Weiß schreiben
		tft.setCursor(40, 106);
		tft.setTextColor(ST7735_WHITE);
		if (tempSOLL < 100)
		{
			tft.print(" ");
		}
		if (tempSOLL < 10)
		{
			tft.print(" ");
		}
		tft.print(tempSOLL);
		tempSOLL_OLD = tempSOLL;
	}
}

void setPwmFrequency(int pin, int divisor)
{
	byte mode;
	if (pin == 5 || pin == 6 || pin == 9 || pin == 10)
	{
		switch (divisor)
		{
		case 1:
			mode = 0x01;
			break;
		case 8:
			mode = 0x02;
			break;
		case 64:
			mode = 0x03;
			break;
		case 256:
			mode = 0x04;
			break;
		case 1024:
			mode = 0x05;
			break;
		default:
			return;
		}

		if (pin == 5 || pin == 6)
		{
			TCCR0B = TCCR0B & 0b11111000 | mode;
		}
		else
		{
			TCCR1B = TCCR1B & 0b11111000 | mode;
		}
	}
	else if (pin == 3 || pin == 11)
	{
		switch (divisor)
		{
		case 1:
			mode = 0x01;
			break;
		case 8:
			mode = 0x02;
			break;
		case 32:
			mode = 0x03;
			break;
		case 64:
			mode = 0x04;
			break;
		case 128:
			mode = 0x05;
			break;
		case 256:
			mode = 0x06;
			break;
		case 1024:
			mode = 0x07;
			break;
		default:
			return;
		}

		TCCR2B = TCCR2B & 0b11111000 | mode;
	}
}
