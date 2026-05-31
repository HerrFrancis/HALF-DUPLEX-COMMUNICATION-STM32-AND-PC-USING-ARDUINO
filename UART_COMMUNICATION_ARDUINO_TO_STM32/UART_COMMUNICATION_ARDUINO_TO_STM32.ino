// NRF24 libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Defines
#define PAYLOAD_SIZE 32 // Number of bytes transferred in each operation

// Pin definitions
const int ARDUINO_READY_PIN = 3; // This pin will be set to high if the arduino is ready to receive data through UART
const int RF24_FAULT_LED_PIN = 4; // This LED will be turned on if RF24 failed to initialize properly
const int STATUS_LED_PIN = 5; // LED used for miscellaneous debugging 
const int RF24_CE_PIN = 7; // Chip Enable pin connected to RF24
const int RF24_CSN_PIN = 8; // Chip Selection pin connected to RF24

// Prototypes
void readPayloadSTM32(void); // Reads data from STM32
void writePayloadSTM32(void); // Writes data to STM32
void readPayloadRF24(void); // Reads data from RF24
void writePayloadRF24(void); // Writes data to RF24
void copyBuffer(uint8_t * buffer1, uint8_t *buffer2); // Copies buffer1 to buffer2 
void setRF24Read(void); // Configures RF24 to receive mode
void setRF24Write(void); // Configures RF24 to transmit mode

// Objects
RF24 radio(RF24_CE_PIN, RF24_CSN_PIN); // CE, CSN

// Global variables/buffers
const byte addressRx[5] = {0x99, 0x99, 0x99, 0x99, 0x99}; // Address used to receive data
const byte addressTx[5] = {0x77, 0x77, 0x77, 0x77, 0x77}; // Adress used to transmit data
uint8_t RF24_rxBuffer[PAYLOAD_SIZE]; // Data read from NRF24
uint8_t RF24_txBuffer[PAYLOAD_SIZE]; // Data to be transmitted to NRF24
uint8_t STM32_rxBuffer[PAYLOAD_SIZE]; // Data read from STM32
uint8_t STM32_txBuffer[PAYLOAD_SIZE]; // Data to be transmitted to STM32

// Debugging variables
// uint8_t counter = 0;

void setup() 
{
  pinMode(RF24_FAULT_LED_PIN, OUTPUT);
  pinMode(ARDUINO_READY_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(ARDUINO_READY_PIN, LOW); // Not ready yet for UART with STM32
  digitalWrite(RF24_FAULT_LED_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, LOW);
  if (!radio.begin()) // If RF24 failed to initialize properly...
  {
    digitalWrite(RF24_FAULT_LED_PIN, HIGH); // To indicate the user
    while(true);
  }
  radio.setDataRate(RF24_2MBPS);
  radio.setRetries(1, 15);
  radio.setChannel(42);
  radio.setPALevel(RF24_PA_MIN);
  setRF24Read(); // Idle mode will be receiver
  Serial.begin(115200, SERIAL_8N1); // 8 data bits, No parity, 1 stop bit at 115200 bit/s
  delay(100); // Arduino is too slow
  digitalWrite(ARDUINO_READY_PIN, HIGH); // Ready for UART with STM32
}

void loop(void) 
{
  if (Serial.available() >= PAYLOAD_SIZE) // Check if STM32 has sent info
  {
    readPayloadSTM32();
    copyBuffer(STM32_rxBuffer, RF24_txBuffer);
    setRF24Write(); // RF24 is only set to transmitter mode if needed
    writePayloadRF24();
    setRF24Read(); // Now it's configured back to receiver mode
  }
  if (radio.available()) // Check if RF24 has received info
  {
    readPayloadRF24();
    if (strcmp(RF24_rxBuffer, "LED_TOGGLE") == 0) // Check if the received payload matches the designated command
    {
      digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN));
    }
    copyBuffer(RF24_rxBuffer, STM32_txBuffer);
    writePayloadSTM32();
  }
}


// Custom functions
void readPayloadSTM32(void) // Reads data from STM32
{
  if (Serial.available()) // Checks that there is indeed data to be received
  {
    for (uint8_t i = 0; i < PAYLOAD_SIZE; i++) 
    {
      STM32_rxBuffer[i] = Serial.read();
    }
  }
}

void writePayloadSTM32(void) // Writes data to STM32
{
  for (uint8_t i = 0; i < PAYLOAD_SIZE; i++) 
  {
    Serial.write(STM32_txBuffer[i]);
  }
}

void readPayloadRF24(void) // Reads data from RF24
{
  radio.read(RF24_rxBuffer, PAYLOAD_SIZE);
}

void writePayloadRF24(void) // Writes data to RF24
{
  radio.write(RF24_txBuffer, PAYLOAD_SIZE);
}

void copyBuffer(uint8_t * buffer1, uint8_t *buffer2) // Copies buffer1 to buffer2 
{
  for (uint8_t i = 0; i < PAYLOAD_SIZE; i++)
  {
    buffer2[i] = buffer1[i];
  }
}

void setRF24Read(void) // Configures RF24 to receive mode
{
  radio.openReadingPipe(0, addressRx);
  radio.startListening();
}

void setRF24Write(void) // Configures RF24 to transmit mode
{
  radio.openWritingPipe(addressTx);
  radio.stopListening();
}

