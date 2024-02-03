/*
 * DC Current and Voltage Meter - Basic Code
 * - Use one INA219, an i2c current monitor from TI
 * -------------------------------------------------
 * Medidor corrente e tensão DC - Código Básico
 * - Utiliza um modulo INA219, um monitor de corrente i2c com um ADC de 12 bit, capaz de monitorar Tensão e Corrente elétrica.
 * - Utiliza um cartão SD para como armazenamento secundario dos registros de leitura.
 * - Utiliza um display LCD 16x2 para vizualização em tempo real da leitura, com Tensao e Corrente elétrica.
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h> // lcd i2c lib
#include <Adafruit_INA219.h>   // ina219 lib
#include <SPI.h>
#include <SD.h>
#include <stdio.h>
#include <time.h>

// Define some constants used in code
// LCD
const int LCD_addr = 0x27; // LCD i2c address
const int LCD_chars = 16;  // number of characters
const int LCD_lines = 2;   // number of lines
// INA219
const int INA_addr = 0x40; // INA219 address

// set the LCD address
LiquidCrystal_I2C lcd(LCD_addr, LCD_chars, LCD_lines);

// set the INA219 adress
Adafruit_INA219 ina219(INA_addr);

// global variables
float shuntvoltage = 0.0;
float loadvoltage = 0.0;
float busvoltage = 0.0;
float current = 0.0;
float samples = 0.0;
float average = 0.0;
float power = 0.0;
int samp_quantity = 100;

unsigned long time_now = 0;

// variable to check if the SD card, Serial and INA219 are OK
bool sdCardOK = false;
bool ina219OK = false;
bool serialOK = false;

// File to log data
const char *fileName = "logFile.txt";

// Prototype of functions
void INA219_Read_Values(void);
void LCD_Update(void);
void Serial_Send(void);
void LOG_Write(String msg);

// setup the pins and initialize the modules
void setup()
{
    // configure the serial to 9600bps
    Serial.begin(9600);

    if (Serial.available()) {
        serialOK = true;
        LOG_Write("Serial is open");
    }

    // initialize the SD card, CS in pin 4
    if (SD.begin(4))
    {
        sdCardOK = true;
        LOG_Write("SD card is ready to use.");
    }

    // initialize the INA219
    if (ina219.begin())
    {
        ina219OK = true;
        if (sdCardOK)
        {
            LOG_Write("INA219 is ready to use.");
        }
    }

    // initialize the LCD
    lcd.init();
    lcd.clear();
    lcd.backlight(); // Print a message to the LCD.
    lcd.setCursor(0, 0); // set the cursor and write some text
}

void loop()
{
    // read values from INA219
    INA219_Read_Values();

    // send data over the serial
    Serial_Send();

    // update the LCD
    LCD_Update();

    // wait 0.5s before next loop
    delay(500);
}

/*
 * Functions
 */

// Read the values from INA219
void INA219_Read_Values(void)
{
    // get the time now
    time_now = millis();

    shuntvoltage = ina219.getShuntVoltage_mV();
    busvoltage = ina219.getBusVoltage_V();
    current = ina219.getCurrent_mA();
    power = ina219.getPower_mW();

    loadvoltage = busvoltage + (shuntvoltage / 1000);

    if (current < 0)
    {
        current = 0;
    }
    if (busvoltage < 0)
    {
        busvoltage = 0;
    }

    // Send data to log
    LOG_Write(String(time_now) + ", " + String(shuntvoltage) + ", " + String(busvoltage) + ", " + String(loadvoltage) + ", " + String(current) + ", " + String(power));
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

    // print the current value
    if (current >= 1000)
    {
        lcd.print(current / 1000);
        lcd.print("A      ");
    }
    else
    {
        lcd.print(current);
        lcd.print("mA     ");
    }
}

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
    Serial.print(current);
    Serial.print(", ");
    Serial.print("power:");
    Serial.print(power);
    Serial.print("\n");
}

void LOG_Write(String msg)
{
    File dataFile = SD.open(fileName, FILE_WRITE);

    if (dataFile) {
        dataFile.println(msg);
        dataFile.close();
    }
}
