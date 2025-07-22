#include <reg51.h>
#include <stdutils.h>

// UART related definitions
#define UART_BAUDRATE 9600UL

// Define button pin (connect a push button to this pin)
sbit STORE_BUTTON = P1^0;  // Button to display stored codes
sbit LED = P1^2;

// Variables for IR remote decoding
uint32_t bitPattern=0, newKey=0;
uint8_t timerValue;
uint8_t msCount=0;
char pulseCount=0;

// Storage for IR codes (up to 5 codes)
#define MAX_STORED_CODES 5
uint32_t storedCodes[MAX_STORED_CODES];
uint8_t codeCount = 0;
uint8_t currentCodeIndex = 0;

// UART functions
void UART_Init(void)
{
    TMOD |= 0x20;  // Timer 1, mode 2 (8-bit auto-reload)
    TH1 = 0xFD;    // Value for 9600 baud rate at 11.0592MHz
    SCON = 0x50;   // Mode 1, reception enabled
    TR1 = 1;       // Start timer 1
}

void UART_TxChar(char ch)
{
    SBUF = ch;     // Load the character to be transmitted
    while(TI==0);  // Wait until transmission is complete
    TI = 0;        // Clear the transmission flag
}

void UART_TxString(char *str)
{
    while(*str)
    {
        UART_TxChar(*str++);
    }
}

void UART_TxHexByte(uint8_t byte)
{
    char hexChars[] = "0123456789ABCDEF";
    UART_TxChar(hexChars[(byte >> 4) & 0x0F]);
    UART_TxChar(hexChars[byte & 0x0F]);
}

void UART_TxHex32(uint32_t value)
{
    UART_TxString("0x");
    UART_TxHexByte((uint8_t)(value >> 24));
    UART_TxHexByte((uint8_t)(value >> 16));
    UART_TxHexByte((uint8_t)(value >> 8));
    UART_TxHexByte((uint8_t)value);
}

// Function to store a new IR code
void storeCode(uint32_t ircode)
{
    uint8_t i;
    
    if(codeCount < MAX_STORED_CODES)
    {
        storedCodes[codeCount] = ircode;
        UART_TxString("\r\nStored Code ");
        UART_TxChar('0' + codeCount);
        UART_TxString(": ");
        UART_TxHex32(ircode);
        codeCount++;
    }
    else
    {
        // Circular buffer behavior - overwrite oldest code
        for(i=0; i<MAX_STORED_CODES-1; i++)
        {
            storedCodes[i] = storedCodes[i+1];
        }
        storedCodes[MAX_STORED_CODES-1] = ircode;
        UART_TxString("\r\nStored Code (overwrite): ");
        UART_TxHex32(ircode);
    }
}

// Function to display all stored codes
void displayStoredCodes(void)
{
    uint8_t i;
    
    if(codeCount > 0)
    {
        UART_TxString("\r\n--- Stored IR Codes ---");
        for(i=0; i<codeCount; i++)
        {
            UART_TxString("\r\nCode ");
            UART_TxChar('0' + i);
            UART_TxString(": ");
            UART_TxHex32(storedCodes[i]);
        }
        UART_TxString("\r\n--- End of List ---");
    }
    else
    {
        UART_TxString("\r\nNo IR codes stored yet!");
    }
}

// Timer 0 ISR for IR signal timing
void timer0_isr() interrupt 1
{
    if(msCount < 50)
        msCount++;
    TH0 = 0xFC;   // Reload the timer value for 1ms Delay
    TL0 = 0x67;
}

// External interrupt 0 ISR for IR signal detection
void externalIntr0_ISR() interrupt 0
{
    timerValue = msCount;
    msCount = 0;
    TH0 = 0xFC;  // Reload the timer value for 1ms Delay
    TL0 = 0x67;
    pulseCount++;
    
    if((timerValue >= 50)) // If pulse width is greater than 50ms, this marks the SOF
    { 
        pulseCount = -2; // First 2 counts need to be skipped hence pulse count is set to -2 
        bitPattern = 0;
    }
    else if((pulseCount >= 0) && (pulseCount < 32)) // Accumulate the bit values between 0-31
    {  
        if(timerValue >= 2) // Pulse width greater than 2ms is considered as LOGIC1
        {
            bitPattern |= (uint32_t)1 << (31 - pulseCount);
        }
    }
    else if(pulseCount >= 32) // This marks the End of frame as 32 pulses are received
    {
        newKey = bitPattern;  // Copy the newKey(pattern) and set the pulse count to 0
        pulseCount = 0;
    }
}

void main()
{
    uint8_t lastButtonState = 1;  // Assume button is not pressed initially
    uint8_t currentButtonState;
    
    // Initialize UART
    UART_Init();
    UART_TxString("\r\nIR Remote Receiver\r\n");
    UART_TxString("Initialization Done\r\n");
    UART_TxString("Press button to display stored codes\r\n");
    
    // Timer 0 setup for IR timing
    TMOD |= 0x01;  // Timer0 MODE1 (16-bit timer)
    TH0 = 0xFC;    // Timer value for 1ms at 11.0592MHz clock
    TL0 = 0x67;
    TR0 = 1;       // Start the Timer
    ET0 = 1;       // Enable the Timer0 Interrupt
    
    // External interrupt setup for IR signal
    IT0 = 1;       // Configure INT0 falling edge interrupt
    EX0 = 1;       // Enable the INT0 External Interrupt
    
    EA = 1;        // Enable the Global Interrupt bit
    
    while(1)
    {
        if(newKey != 0)  // New IR code received
        {  
            UART_TxString("\r\nReceived Code: ");
            UART_TxHex32(newKey);
            
            // Store the received code
            storeCode(newKey);
            newKey = 0;  // Clear the newKey value for next cycle
            
            LED = !LED;  // Toggle LED to indicate code received
        }
        
        // Check button state for displaying stored codes
        currentButtonState = STORE_BUTTON;
        if(currentButtonState == 1 && lastButtonState == 0)  // Button pressed (falling edge)
        {
            // Display all stored codes
            displayStoredCodes();
        }
        lastButtonState = currentButtonState;
    }
}