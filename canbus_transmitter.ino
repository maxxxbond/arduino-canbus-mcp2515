#include <Wire.h>
#include <SPI.h>
#include <mcp2515.h>

// ADXL345 settings
#define ADXL345_ADDRESS (0xA6 >> 1)
#define ADXL345_REGISTER_XLSB 0x32

int accelerometer_data[3];

// Ultrasonic sensor settings
int inputPin = 4;  // define ultrasonic signal receiver pin ECHO to D4 
int outputPin = 5; // define ultrasonic signal transmitter pin TRIG to D5

// CAN settings
struct can_frame canMsg;
MCP2515 mcp2515(10);

// I2C write function
void i2c_write(int address, byte reg, byte data) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

// I2C read function
void i2c_read(int address, byte reg, int count, byte* data) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission();

  Wire.requestFrom(address, count);
  for (int i = 0; i < count; i++) {
    if (Wire.available()) {
      data[i] = Wire.read();
    }
  }
}

// Initialize ADXL345
void init_adxl345() {
  byte data = 0;

  i2c_write(ADXL345_ADDRESS, 0x31, 0x08);   // 13-bit mode, +-2g
  i2c_write(ADXL345_ADDRESS, 0x2D, 0x08);   // Power register
  i2c_write(ADXL345_ADDRESS, 0x1E, 0x00);   // X offset
  i2c_write(ADXL345_ADDRESS, 0x1F, 0x00);   // Y offset
  i2c_write(ADXL345_ADDRESS, 0x20, 0x05);   // Z offset

  i2c_read(ADXL345_ADDRESS, 0x00, 1, &data);
  if (data == 0xE5) {
    Serial.println("Initialization Success");
  } else {
    Serial.println("Initialization Fail");
  }
}

// Read data from ADXL345
void read_adxl345() {
  byte bytes[6];
  memset(bytes, 0, sizeof(bytes));

  i2c_read(ADXL345_ADDRESS, ADXL345_REGISTER_XLSB, 6, bytes);

  for (int i = 0; i < 3; i++) {
    accelerometer_data[i] = ((int)bytes[2 * i + 1] << 8) | (int)bytes[2 * i];
    if (accelerometer_data[i] & 0x2000) {
      accelerometer_data[i] |= 0xC000;
    }
  }
}

// Initialize ultrasonic sensor pins
void init_ultrasonic() {
  pinMode(inputPin, INPUT); 
  pinMode(outputPin, OUTPUT);
}

// Read data from ultrasonic sensor
int read_ultrasonic() {
  digitalWrite(outputPin, LOW); 
  delayMicroseconds(2);
  digitalWrite(outputPin, HIGH);
  delayMicroseconds(10); 
  digitalWrite(outputPin, LOW);

  long duration = pulseIn(inputPin, HIGH); // Read receiver pulse time 
  int distance = duration / 58; // Transform pulse time to distance 
  return distance;
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  SPI.begin();

  init_adxl345();
  init_ultrasonic();

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();
}

void loop() {
  read_adxl345();
  int distance = read_ultrasonic();

  canMsg.can_id  = 0x036; // CAN id as 0x036
  canMsg.can_dlc = 7;     // CAN data length as 7
  canMsg.data[0] = (byte)(distance & 0xFF); // Distance
  canMsg.data[1] = (byte)(accelerometer_data[0] & 0xFF);         // Accelerometer X low byte
  canMsg.data[2] = (byte)((accelerometer_data[0] >> 8) & 0xFF);  // Accelerometer X high byte
  canMsg.data[3] = (byte)(accelerometer_data[1] & 0xFF);         // Accelerometer Y low byte
  canMsg.data[4] = (byte)((accelerometer_data[1] >> 8) & 0xFF);  // Accelerometer Y high byte
  canMsg.data[5] = (byte)(accelerometer_data[2] & 0xFF);         // Accelerometer Z low byte
  canMsg.data[6] = (byte)((accelerometer_data[2] >> 8) & 0xFF);  // Accelerometer Z high byte

  mcp2515.sendMessage(&canMsg);

  delay(100);
}
