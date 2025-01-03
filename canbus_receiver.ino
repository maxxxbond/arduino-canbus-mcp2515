#include <SPI.h> 
#include <mcp2515.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

struct can_frame canMsg;

MCP2515 mcp2515(10); // SPI CS Pin 10

LiquidCrystal_I2C lcd(0x27, 20, 4); // Set the LCD address to 0x27 for a 20 chars and 4 line display

void setup() {
  Serial.begin(9600); // Begins Serial Communication at 9600 baudrate
  SPI.begin();

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode(); // Sets CAN at normal mode

  lcd.init();
  lcd.backlight();

  // Display static text
  lcd.setCursor(0, 0);
  lcd.print("Dist: ");
  lcd.setCursor(0, 1);
  lcd.print("Accel X: ");
  lcd.setCursor(0, 2);
  lcd.print("Accel Y: ");
  lcd.setCursor(0, 3);
  lcd.print("Accel Z: ");
}

void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    int distance = canMsg.data[0];
    // Сonverting two-bit values to integer format
    int accel_x = (canMsg.data[2] << 8) | canMsg.data[1];
    int accel_y = (canMsg.data[4] << 8) | canMsg.data[3];
    int accel_z = (canMsg.data[6] << 8) | canMsg.data[5];

    // Update the LCD with new values
    lcd.setCursor(6, 0);
    lcd.print(distance);
    lcd.print(" cm  "); // Add some padding to clear previous value

    lcd.setCursor(9, 1);
    lcd.print(accel_x);
    lcd.print("     "); // Add some padding to clear previous value

    lcd.setCursor(9, 2);
    lcd.print(accel_y);
    lcd.print("     "); //

    lcd.setCursor(9, 3);
    lcd.print(accel_z);
    lcd.print("     "); // Add some padding to clear previous value

    delay(100);
  }
}