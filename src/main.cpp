/*
 * DC Current and Voltage Meter - Basic Code
 * - Use one INA219, an i2c current monitor from TI
 * with 12 bit ADC, capable to monitor Voltage and
 * Current
 *
 * -------------------------------------------------
 * Medidor corrente e tensão DC - Código Básico
 * - Utiliza um modulo INA219, um monitor de corrente i2c
 * com um ADC de 12 bit, capaz de monitorar Tensão
 * e Corrente
 *
 * developed by: Haroldo Amaral
 * 2017/03/26 - v 1.0
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // lcd i2c lib
#include <Adafruit_INA219.h>   // ina219 lib
#include <SPI.h>
#include <SD.h>
#include <stdio.h>

// Define some constants used in code - LED
const int LED1 = 2; // the number of the LED pin
const int LED2 = 3; // the number of the LED pin
const int ON = 1;   // on state
const int OFF = 0;  // off state

// Define some constants used in code - LCD
const int LCD_addr = 0x27; // LCD i2c address
const int LCD_chars = 16;  // number of characters
const int LCD_lines = 2;   // number of lines

// Define some constants used in code - INA219
const int INA_addr = 0x40; // INA219 address

// set the INA219 to address 0x40
Adafruit_INA219 ina219(INA_addr);

// set the LCD address to 0x3F for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(LCD_addr, LCD_chars, LCD_lines);

// global variables
// variables iNA219
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float loadvoltage = 0;

// variables to calculate the average
float samples = 0.0;
float average = 0.0;
int samp_quantity = 100;

// variable to check if the SD card and INA219 are OK
bool sdCardOK = false;
bool ina219OK = false;

// File to log data
const char *fileName = "logFile.txt";

// Prototype of functions
void Read_INA219_Values(void);
void LCD_Update(void);
void Serial_Send(void);
void LOG(String msg);

// setup the pins and initialize the modules
void setup()
{
    // configure LED pins
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);

    // configure the serial to 9600bps
    Serial.begin(9600);

    // initialize the SD card, CS in pin 4
    if (SD.begin(4))
    {
        sdCardOK = true;
        Serial.println("SD card is ready to use.");
        LOG("SD card is ready to use.");
    }
    else
    {
        sdCardOK = false;
        Serial.println("SD card is not ready to use.");
    }

    // initialize the INA219
    if (ina219.begin())
    {
        ina219OK = true;
        if (sdCardOK)
        {
            Serial.println("ina219 is ready to use.");
            LOG("ina219 is ready to use.");
        }
    }
    else
    {
        ina219OK = false;
        if (sdCardOK)
        {
            Serial.println("ina219 is not ready to use.");
        }
    }

    // initialize the lcd
    lcd.init();
    lcd.clear();
    // Print a message to the LCD.
    lcd.backlight();
    // set the cursor and write some text
    lcd.setCursor(0, 0);
}

/*
 * MAIN LOOP
 * - change the led state, measure the values and update LCD
 */

void loop()
{
    // read values from INA219
    Read_INA219_Values();

    // send data over the serial
    Serial_Send();

    // update the LCD
    LCD_Update();
}

/*
 * Functions
 */

// Read the values from INA219
void Read_INA219_Values(void)
{
    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();

    loadvoltage = busvoltage + (shuntvoltage / 1000);

    // calculate the average of samples
    for (int i = 0; i < samp_quantity; i++)
    {
        current_mA = ina219.getCurrent_mA();
        samples += current_mA;

        delay(5);
    }

    average = samples / samp_quantity;
    samples = 0.0;

    // update the current value with the average
    current_mA = average;

    if (current_mA < 0)
    {
        current_mA = 0;
    }

    if (busvoltage < 0)
    {
        busvoltage = 0;
    }

    // Send data to log
    LOG(String(shuntvoltage) + ", " + String(busvoltage) + ", " + String(loadvoltage) + ", " + String(current_mA));
}

// update the LCD with values
void LCD_Update(void)
{
    lcd.setCursor(0, 0);
    lcd.print("U: ");
    lcd.print(busvoltage);
    lcd.print("V");

    lcd.setCursor(0, 1);
    lcd.print("I: ");
    if (current_mA < 10)
    {
        lcd.print(" ");
    }
    lcd.print(current_mA);
    lcd.print("mA");
}

// Send data over the serial
// envia os valores pela serial
void Serial_Send(void)
{
    Serial.print("bus_voltage:");
    Serial.print(busvoltage);
    Serial.print(", ");
    Serial.print("load_voltage:");
    Serial.print(loadvoltage);
    Serial.print(", ");
    Serial.print("shunt_voltage:");
    Serial.print(shuntvoltage);
    Serial.print(", ");
    Serial.print("current:");
    Serial.print(current_mA);
    Serial.println();
}

void LOG(String msg)
{
    File dataFile = SD.open(fileName, FILE_WRITE);

    if (!dataFile)
    {
        Serial.println("error opening file");
    }
    else
    {
        Serial.print("Writing to log file... ");
        Serial.println(msg);
        Serial.println();

        dataFile.println(msg);
        dataFile.close();
    }
}
