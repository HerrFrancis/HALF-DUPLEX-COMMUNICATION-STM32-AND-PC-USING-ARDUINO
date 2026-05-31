// NRF24 libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Defines
#define PAYLOAD_SIZE 32 // Number of bytes transferred in each operation

// Pin definitions
const int RF24_CE_PIN = 7; // Chip Enable pin connected to RF24
const int RF24_CSN_PIN = 8; // Chip Selection pin connected to RF24

// Prototypes
void writePayloadPC(void); // Writes data to PC
void readPayloadPC(void); // Reads data from PC
void writePayloadRF24(void); // Writes data to RF24
void readPayloadRF24(void); // Reads data from RF24
void setRF24Read(void); // Configures RF24 to receive mode
void setRF24Write(void); // Configures RF24 to transmit mode
void copyBuffer(uint8_t * buffer1, uint8_t *buffer2); // Copies buffer1 to buffer2 

// Objects
RF24 radio(RF24_CE_PIN, RF24_CSN_PIN); // CE, CSN

// Global variables/buffers
const byte addressRx[5] = {0x77, 0x77, 0x77, 0x77, 0x77}; // Address used to receive data
const byte addressTx[5] = {0x99, 0x99, 0x99, 0x99, 0x99}; // Adress used to transmit data
uint8_t RF24_rxBuffer[PAYLOAD_SIZE]; // Data read from NRF24
uint8_t RF24_txBuffer[PAYLOAD_SIZE]; // Data to be transmitted to NRF24
uint8_t PC_rxBuffer[PAYLOAD_SIZE]; // Data read from PC
uint8_t PC_txBuffer[PAYLOAD_SIZE]; // Data to be transmitted to PC

void setup() 
{
  Serial.begin(115200);
  if (!radio.begin()) // If RF24 failed to initialize properly...
  {
    Serial.println("NRF24 malfunction, please check connections"); // To indicate the user
    while(true);
  }
  radio.setChannel(42);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_2MBPS); 
  radio.setRetries(1, 15);
  setRF24Read(); // Idle mode will be receiver
  Serial.println("Ready");
}

void loop(void) 
{
  if (Serial.available()) // Detects if PC has sent info
  {
    delay(10); // Wait until all data bits arrive from PC
    readPayloadPC();
    copyBuffer(PC_rxBuffer, RF24_txBuffer);
    setRF24Write(); // RF24 is only set to transmitter mode if needed
    writePayloadRF24();
    setRF24Read(); // Check if RF24 has received info
  }
  if (radio.available()) // Detects if data has been received by RF24
  {
    readPayloadRF24();
    copyBuffer(RF24_rxBuffer, PC_txBuffer);
    writePayloadPC();
  }
}

// Custom functions
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

void copyBuffer(uint8_t * buffer1, uint8_t *buffer2) // Copies buffer1 to buffer2 
{
  for (uint8_t i = 0; i < PAYLOAD_SIZE; i++)
  {
    buffer2[i] = buffer1[i];
  }
}

void writePayloadPC(void) // Writes data to PC
{
  for (uint8_t i = 0; i < PAYLOAD_SIZE; i++)
  {
    Serial.print((char) PC_txBuffer[i]);
  }
  Serial.println();
}

void readPayloadPC(void) // Reads data from PC
{
  for (uint8_t i = 0; i < PAYLOAD_SIZE; i++) 
  {
    if (Serial.available()) // Checks that there is indeed data to be received
    {
      PC_rxBuffer[i] = Serial.read();
    }
    else
    {
      PC_rxBuffer[i] = 0x00;
    }
  }
}

void writePayloadRF24(void) // Writes data to RF24
{
  radio.write(RF24_txBuffer, PAYLOAD_SIZE);
}

void readPayloadRF24(void) // Reads data from RF24
{
  radio.read(RF24_rxBuffer, PAYLOAD_SIZE);
}