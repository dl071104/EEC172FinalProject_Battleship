#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "simplelink.h"
// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_nvic.h"
#include "hw_memmap.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "hw_apps_rcm.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
//#include "prcm.h"
#include "gpio.h"
#include "utils.h"
#include "systick.h"
#include "oled_test.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1351.h"

// Common interface includes
#include "uart_if.h"
#include "i2c_if.h"
#include "spi.h"
#include "uart.h"
#include "gpio_if.h"
#include "common.h"
#include "pin_mux_config.h"
#include "utils/network_utils.h"

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
//AWS
#define DATE                12    /* Current Date */
#define MONTH               3     /* Month 1-12 */
#define YEAR                2025  /* Current year */
#define HOUR                12    /* Time - hours */
#define MINUTE              27    /* Time - minutes */
#define SECOND              0     /* Time - seconds */


#define APPLICATION_NAME      "SSL"
#define APPLICATION_VERSION   "SQ24"
#define SERVER_NAME           "a30ebqb9zrghmt-ats.iot.us-east-1.amazonaws.com" // CHANGE ME
#define GOOGLE_DST_PORT       8443


#define POSTHEADER "POST /things/Allison_CC3200_Board/shadow HTTP/1.1\r\n"             // CHANGE ME
#define GETHEADER "GET /things/Allison_CC3200_Board/shadow HTTP/1.1\r\n"             // CHANGE ME
#define HOSTHEADER "Host: a30ebqb9zrghmt-ats.iot.us-east-1.amazonaws.com\r\n"  // CHANGE ME //changed
#define CHEADER "Connection: Keep-Alive\r\n"
#define CTHEADER "Content-Type: application/json; charset=utf-8\r\n"
#define CLHEADER1 "Content-Length: "
#define CLHEADER2 "\r\n\r\n"

// JSON structure for AWS IoT Device Shadow update
#define DATASTART "{\r\n" \
"  \"state\": {\r\n" \
"    \"desired\": {\r\n" \
//                 "      \"var\": \""
#define DATAEND   "  }\r\n" \
"}\r\n\r\n"


// JSON structure for AWS SNS message format
#define MESSAGESTART  "  \"default\": \""
#define MESSAGEMID "\",\r\n" \
  "  \"email\": \""
#define MESSAGEEND "\"\r\n" \
 "}\r\n"

//AWS end
#define SPI_IF_BIT_RATE  100000
#define TR_BUFF_SIZE     100

#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
//BUTTON DECLARATION
#define ZERO           0xFD02FF00
#define ONE            0xFD027F80
#define TWO            0xFD02BF40
#define THREE          0xFD023FC0
#define FOUR           0xFD02DF20
#define FIVE           0xFD025FA0
#define SIX            0xFD029F60
#define SEVEN          0xFD021FE0
#define EIGHT          0xFD02EF10
#define NINE           0xFD026F90
#define LAST           0xFD0217E8
#define MUTE           0xFD02F708

#define GRID_SIZE 7    // 7x7 grid
#define SQUARE_SIZE 8   // Each square is 3x3 pixels
#define GRID_START_X 10  // Top-left grid start
#define GRID_START_Y 10
#define GRID2_START_X 70 // Bottom-right grid start
#define GRID2_START_Y 70

unsigned int abc = 0;
unsigned int def = 0;
unsigned int ghi = 0;
unsigned int jkl = 0;
unsigned int mno = 0;
unsigned int pqrs = 0;
unsigned int tuv = 0;
unsigned int wxyz = 0;
char message[1000];
char UART_Recv[1000];
char username[1000];
char firstship[1000];
char secondship[1000];
char thirdship[1000];
int message_int = 0;
unsigned int recvIndex = 0;
int counter = 0;
int turn_counter = 0; //TEST
//AWS
int local_username_index;
int Win = 0;
char round[100] = "         Turns to finish the game: ";
char RoundNumber[2];
char Local_player[1000] = " Local Player Username: ";
char Remote_player[1000] = " Remote Player Username: ";
char space[100] = "      ";
char winnerstatement[1000] = "              Winner: ";
char winner_user[1000];
char Remote_username[1000];
char round_to_email[2];
//AWS end
int x_coor1_2ship = 0;
int y_coor1_2ship = 0;
int x_coor2_2ship = 0;
int y_coor2_2ship = 0;

int x_coor1_3ship = 0;
int y_coor1_3ship = 0;
int x_coor2_3ship = 0;
int y_coor2_3ship = 0;

int x_coor1_4ship = 0;
int y_coor1_4ship = 0;
int x_coor2_4ship = 0;
int y_coor2_4ship = 0;

int x_attack_coor = 0;
int y_attack_coor = 0;


char ship_locations[10][1000];

char hit[] = "HIT";
char miss[] = "MISS";
char gameover[] = "XXX";
char attack_coordinate[1000];

extern void (* const g_pfnVectors[])(void);

volatile unsigned long IR_intcount;

volatile unsigned char IR_intflag;

volatile unsigned int SIGNAL[32];
volatile unsigned int START_FLAG;
volatile unsigned int CHECK_FLAG;

// some helpful macros for systick
// the cc3200's fixed clock frequency of 80 MHz
// note the use of ULL to indicate an unsigned long long constant
#define SYSCLKFREQ 80000000ULL

// macro to convert ticks to microseconds
#define TICKS_TO_US(ticks) \
    ((((ticks) / SYSCLKFREQ) * 1000000ULL) + \
    ((((ticks) % SYSCLKFREQ) * 1000000ULL) / SYSCLKFREQ))\
// macro to convert microseconds to ticks
#define US_TO_TICKS(us) ((SYSCLKFREQ / 1000000ULL) * (us))

// systick reload value set to 40ms period
// (PERIOD_SEC) * (SYSCLKFREQ) = PERIOD_TICKS
#define SYSTICK_RELOAD_VAL 3200000UL

// track systick counter periods elapsed
// if it is not 0, we know the transmission ended
volatile int systick_cnt = 0;

extern void (* const g_pfnVectors[])(void);

//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************

// an example of how you can use structs to organize your pin settings for easier maintenance
typedef struct PinSetting {
    unsigned long port;
    unsigned int pin;
} PinSetting;

static const PinSetting IR = { .port = GPIOA0_BASE, .pin = 0x40};


//*****************************************************************************
//                      LOCAL FUNCTION PROTOTYPES                           
//*****************************************************************************
static int set_time();
static void BoardInit(void);
static int http_post(int);
static int http_get(int);
//*****************************************************************************
//                      LOCAL FUNCTION DEFINITIONS                         
//*****************************************************************************
void ABC() { // functions to cycle through the letters on the num keypad
    message_int--;
    abc++;
    def = 0;
    ghi = 0;
    jkl = 0;
    mno = 0;
    pqrs = 0;
    tuv = 0;
    wxyz = 0;
    if(abc == 1) {
        message[message_int] = 'B';
        Message("\bB");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(abc == 2) {
        message[message_int] = 'C';
        Message("\bC");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
}
void DEF() {
    message_int--;
    abc = 0;
    def++;
    ghi = 0;
    jkl = 0;
    mno = 0;
    pqrs = 0;
    tuv = 0;
    wxyz = 0;
    if(def == 1) {
        message[message_int] = 'E';
        Message("\bE");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(def == 2) {
        message[message_int] = 'F';
        Message("\bF");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
}
void GHI() {
    message_int--;
    abc = 0;
    def = 0;
    ghi++;
    jkl = 0;
    mno = 0;
    pqrs = 0;
    tuv = 0;
    wxyz = 0;
    if(ghi == 1) {
        message[message_int] = 'H';
        Message("\bH");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(ghi == 2) {
        message[message_int] = 'I';
        Message("\bI");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
}
void JKL() {
    message_int--;
    abc = 0;
    def = 0;
    ghi = 0;
    jkl++;
    mno = 0;
    pqrs = 0;
    tuv = 0;
    wxyz = 0;
    if(jkl == 1) {
        message[message_int] = 'K';
        Message("\bK");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(jkl == 2) {
        message[message_int] = 'L';
        Message("\bL");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
}

void MNO() {
    message_int--;
    abc = 0;
    def = 0;
    ghi = 0;
    jkl = 0;
    mno++;
    pqrs = 0;
    tuv = 0;
    wxyz = 0;
    if(mno == 1) {
        message[message_int] = 'N';
        Message("\bN");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(mno == 2) {
        message[message_int] = 'O';
        Message("\bO");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }

}
void PQRS() {
    message_int--;
    abc = 0;
    def = 0;
    ghi = 0;
    jkl = 0;
    mno = 0;
    pqrs++;
    tuv = 0;
    wxyz = 0;
    if(pqrs == 1) {
        message[message_int] = 'Q';
        Message("\bQ");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(pqrs == 2) {
        message[message_int] = 'R';
        Message("\bR");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(pqrs == 3) {
        message[message_int] = 'S';
        Message("\bS");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
}
void TUV() {
    message_int--;
    abc = 0;
    def = 0;
    ghi = 0;
    jkl = 0;
    mno = 0;
    pqrs = 0;
    tuv++;
    wxyz = 0;
    if(tuv == 1) {
        message[message_int] = 'U';
        Message("\bU");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(tuv == 2) {
        message[message_int] = 'V';
        Message("\bV");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
}
void WXYZ() {
    message_int--;
    abc = 0;
    def = 0;
    ghi = 0;
    jkl = 0;
    mno = 0;
    pqrs = 0;
    tuv = 0;
    wxyz++;
    if(wxyz == 1) {
        message[message_int] = 'X';
        Message("\bX");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(wxyz == 2) {
        message[message_int] = 'Y';
        Message("\bY");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
    else if(wxyz == 3) {
        message[message_int] = 'Z';
        Message("\bZ");
        fillRect(80, 35, 50, 20, BLACK);
        setCursor(80, 35);
        Outstr(message);
    }
}
static void GPIOA0IntHandler(void) {
  //  Report("\t\t\t START_FLAG: %d Time is %d \n\r",START_FLAG, delta_us);
    CHECK_FLAG = 0;
    uint64_t delta = SYSTICK_RELOAD_VAL - SysTickValueGet();
    uint64_t delta_us = TICKS_TO_US(delta);
   // Report("\t\t\t START_FLAG: %d Time is %d \n\r",START_FLAG, delta_us);
    SysTickReset();
  //  printf("COUNT is %d, second is %d \n", IR_intcount, delta_ms);
    unsigned long ulStatus;

    ulStatus= MAP_GPIOIntStatus (GPIOA0_BASE, true);
 //   MAP_GPIOIntClear(GPIO A0_BASE, ulStatus);		// clear interrupts on GPIOA1


    if ((delta_us >= 13000) && (delta_us <= 15000)) // check timing between falling edges
            START_FLAG = 1;
    if (START_FLAG && (IR_intcount<32)){
        if ((delta_us >= 1000) && (delta_us <= 2000))
        SIGNAL[IR_intcount] = 1;
        else if ((delta_us >= 2000) && (delta_us <= 3000))
        SIGNAL[IR_intcount] = 0;
        IR_intcount++;
        //Report("\t\t\t Count %d\n\r", IR_intcount);
    }
    if (IR_intcount==31){ // run the while loop in main once 32 iterations have gone
        START_FLAG = 0;
        CHECK_FLAG = 1;
        IR_intcount = 0;

   // Message("\t\t\t DATA FINISHED \n\r");
    }
    MAP_GPIOIntClear(GPIOA0_BASE, ulStatus);       // clear interrupts on GPIOA1
  //  Report("\t\t\t CHECK_FLAG %d\n\r", CHECK_FLAG);
 //   IR_intflag=1;
}


/**
 * Reset SysTick Counter
 */
void SysTickReset(void) {
    // any write to the ST_CURRENT register clears it
    // after clearing it automatically gets reset without
    // triggering exception logic
    // see reference manual section 3.2.1
    HWREG(NVIC_ST_CURRENT) = 1;

    // clear the global count variable
    systick_cnt = 0;
}
/**
 * SysTick Interrupt Handler
 *
 * Keep track of whether the systick counter wrapped
 */
static void SysTickHandler(void) {
    // increment every time the systick handler fires
    systick_cnt++;
}

/**
 * Initializes SysTick Module
 */
static void SysTickInit(void) {

    // configure the reset value for the systick countdown register
    MAP_SysTickPeriodSet(SYSTICK_RELOAD_VAL);

    // register interrupts on the systick module
    MAP_SysTickIntRegister(SysTickHandler);

    // enable interrupts on systick
    // (trigger SysTickHandler when countdown reaches 0)
    MAP_SysTickIntEnable();

    // enable the systick module itself
    MAP_SysTickEnable();
}
//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void) {
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}

void UART_1_Handler() {
    // UARTIntClear(UARTA1_BASE, UART_INT_RX | UART_INT_RT | UART_INT_TX);
    unsigned long ulStatus = UARTIntStatus(UARTA1_BASE, true);
    UARTIntClear(UARTA1_BASE, ulStatus);
    while (UARTCharsAvail(UARTA1_BASE)) {
        UART_Recv[recvIndex] = UARTCharGet(UARTA1_BASE);
        recvIndex = recvIndex + 1;
    }
/*
    setCursor(0,0);
    Outstr(recv);

    int i;
    for (i = 0; i < recvIndex; i++) {
        recv[i] = '\0';
    }
    recvIndex = 0;
    */

}

void drawGrid(int startX, int startY) {
    int i = 0;
    for (i = 0; i <= GRID_SIZE; i++) {
        // Draw horizontal lines
        drawLine(startX, startY + i * SQUARE_SIZE, startX + GRID_SIZE * SQUARE_SIZE, startY + i * SQUARE_SIZE, WHITE);
        // Draw vertical lines
        drawLine(startX + i * SQUARE_SIZE, startY, startX + i * SQUARE_SIZE, startY + GRID_SIZE * SQUARE_SIZE, WHITE);
    }
}

// Function to label the rows from A to J
void labelGrid(int startX, int startY) {
    char label = 'A';
    int i = 0;
    for (i = 0; i < GRID_SIZE; i++) {
        // Label rows (left side)
        drawChar(startX - 10, startY + i * SQUARE_SIZE + 2, label, WHITE, BLACK, 1);
        // Label columns (top side)
        drawChar(startX + i * SQUARE_SIZE + 2, startY - 10, label, WHITE, BLACK, 1);
        label++;
    }
}

static int set_time() {
    long retVal;

    g_time.tm_day = DATE;
    g_time.tm_mon = MONTH;
    g_time.tm_year = YEAR;
    g_time.tm_sec = HOUR;
    g_time.tm_hour = MINUTE;
    g_time.tm_min = SECOND;

    retVal = sl_DevSet(SL_DEVICE_GENERAL_CONFIGURATION,
                          SL_DEVICE_GENERAL_CONFIGURATION_DATE_TIME,
                          sizeof(SlDateTime),(unsigned char *)(&g_time));

    ASSERT_ON_ERROR(retVal);
    return SUCCESS;
}


//****************************************************************************
//
//! Main function
//!
//! \param none
//! 
//!
//! \return None.
//
//****************************************************************************
int main() {
    long lRetVal = -1;
    unsigned long ulStatus;
    unsigned long receivedData = 0;
    unsigned long lastPressed = 0;
    int i = 0;
    int user_name = 1;//test
    int first_ship = 1; //test
    int second_ship = 1;
    int third_ship = 1;
    int Turns_to_finish = 0;
    local_username_index = 0;
    BoardInit();

    PinMuxConfig();

    InitTerm();

    ClearTerm();

    SysTickInit();
    Report("\t\t\t START HERE \n\r");
    //
    // Register the interrupt handlers
    //
    MAP_GPIOIntRegister(GPIOA0_BASE, GPIOA0IntHandler);


    //
    // Configure falling edge interrupts on SW2 and SW3
    //
    MAP_GPIOIntTypeSet(IR.port, IR.pin, GPIO_FALLING_EDGE);    // SW3


    ulStatus = MAP_GPIOIntStatus(IR.port, false);
    MAP_GPIOIntClear(IR.port, ulStatus);            // clear interrupts on IR

    MAP_PRCMPeripheralReset(PRCM_GSPI);

    MAP_SPIReset(GSPI_BASE);


    MAP_SPIConfigSetExpClk(GSPI_BASE,MAP_PRCMPeripheralClockGet(PRCM_GSPI),
                     SPI_IF_BIT_RATE,SPI_MODE_MASTER,SPI_SUB_MODE_0,
                     (SPI_SW_CTRL_CS |
                     SPI_4PIN_MODE |
                     SPI_TURBO_OFF |
                     SPI_CS_ACTIVEHIGH |
                     SPI_WL_8));


    MAP_SPIEnable(GSPI_BASE);

    MAP_UARTConfigSetExpClk(UARTA1_BASE,PRCMPeripheralClockGet(PRCM_UARTA1),
                      UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                       UART_CONFIG_PAR_NONE));

    UARTIntRegister(UARTA1_BASE, UART_1_Handler);
    UARTIntEnable(UARTA1_BASE,  UART_INT_RX | UART_INT_RT | UART_INT_TX);
    ulStatus = MAP_GPIOIntStatus(GPIOA0_BASE, false);
        MAP_UARTIntClear(GPIOA0_BASE, ulStatus);
    Adafruit_Init();

    // clear global variables
    IR_intcount=0;
    IR_intflag=0;
    START_FLAG = 0;


    // Enable IR interrupts
    MAP_GPIOIntEnable(IR.port, IR.pin);
    /*
//TEST
    int TEST = 10;
    char TESTCH[2];
    sprintf(TESTCH, "%d", TEST);
    Report("\nTurn is :%s", TESTCH);
    UARTCharPut(UARTA1_BASE, TESTCH[0]);
    UARTCharPut(UARTA1_BASE, TESTCH[1]);
    while(1)
    {
        if (GPIOPinRead(GPIOA1_BASE, 0x20) == 0x20) {
                    // Message("SW3 Pressed");
                   Report("SCORE IS %s", UART_Recv);
                }

    }
//TESTEND
 */


    //AWS:
        g_app_config.host = SERVER_NAME;
        g_app_config.port = GOOGLE_DST_PORT;

        //Connect the CC3200 to the local access point
        lRetVal = connectToAccessPoint();
        //Set time so that encryption can be used
        lRetVal = set_time();
        if(lRetVal < 0) {
            UART_PRINT("Unable to set time in the device");
            LOOP_FOREVER();
        }
        //Connect to the website with TLS encryption
        lRetVal = tls_connect();
        if(lRetVal < 0) {
            ERR_PRINT(lRetVal);
        }
        //AWS end


    int game_not_start = 1;
    fillScreen(BLACK);
    setCursor(0, 0);
    Outstr("Welcome to Battleship!");
    setCursor(0, 10);
    Outstr("Press SW3 to");
    setCursor(0, 20);
    Outstr("start the game");
    setCursor(0, 30);
    Outstr("Press 0 to");
    setCursor(0, 40);
    Outstr("read the rules");
    while (game_not_start) {
        if (GPIOPinRead(GPIOA1_BASE, 0x20) == 0x20) {
            // Message("SW3 Pressed");
            game_not_start = 0;
        }

        if (CHECK_FLAG){
            //MAP_GPIOIntDisable(IR.port, IR.pin);
         //   OFF = 1;
            for (i = 0; i < 32; i++) // or with 0 and put into an unsigned long to be checked against the signals from TV remote
            {
                receivedData = (receivedData << 1) | SIGNAL[i+1];
                SIGNAL[i+1] = 0;
            }
            switch (receivedData) {
            case ZERO:
                fillScreen(BLACK);
                setCursor(0,0);
                Outstr("Each player will take");
                setCursor(0,10);
                Outstr("turns entering");
                setCursor(0,20);
                Outstr("coordinates to fire");
                setCursor(0,30);
                Outstr("missiles at each");
                setCursor(0,40);
                Outstr("others ships. The top");
                setCursor(0,50);
                Outstr("left grid is where");
                setCursor(0,60);
                Outstr("ships will be. The");
                setCursor(0,70);
                Outstr("bottom right is where");
                setCursor(0,80);
                Outstr("you can see what you");
                setCursor(0,90);
                Outstr("have hit and missed");
                setCursor(0, 100);
                Outstr("on the enemy's board");
                setCursor(0,110);
                Outstr("Press 1 to go to the");
                setCursor(0,120);
                Outstr("next page of rules");
                break;
            case ONE:
                fillScreen(BLACK);
                setCursor(0,0);
                Outstr("Hits will be marked");
                setCursor(0,10);
                Outstr("in red. Misses will");
                setCursor(0,20);
                Outstr("be marked in green");
                setCursor(0,30);
                Outstr("This will be the same");
                setCursor(0,40);
                Outstr("across both boards");
                setCursor(0,50);
                Outstr("Players will continue");
                setCursor(0,60);
                Outstr("firing missiles until");
                setCursor(0,70);
                Outstr("one side's ships have");
                setCursor(0,80);
                Outstr("all been hit.");
                setCursor(0,90);
                Outstr("When that happens");
                setCursor(0, 100);
                Outstr("the game is over");
                setCursor(0,110);
                Outstr("Press 2 to go to the");
                setCursor(0,120);
                Outstr("next page of rules");
                break;
            case TWO:
                fillScreen(BLACK);
                setCursor(0,0);
                Outstr("You will get to place");
                setCursor(0,10);
                Outstr("choose where your");
                setCursor(0,20);
                Outstr("ships want to be.");
                setCursor(0,30);
                Outstr("Enter them in the");
                setCursor(0,40);
                Outstr("format XYXY, where");
                setCursor(0,50);
                Outstr("the first pair will");
                setCursor(0,60);
                Outstr("be the coordinates");
                setCursor(0,70);
                Outstr("of the head of the");
                setCursor(0,80);
                Outstr("ship and the second");
                setCursor(0,90);
                Outstr("pair will be the");
                setCursor(0, 100);
                Outstr("tail of the ship.");
                setCursor(0,110);
                Outstr("Press 3 to return");
                setCursor(0,120);
                Outstr("to the main menu.");
                break;
            case THREE:
                fillScreen(BLACK);
                setCursor(0, 0);
                Outstr("Welcome to Battleship!");
                setCursor(0, 10);
                Outstr("Press SW3 to");
                setCursor(0, 20);
                Outstr("start the game");
                setCursor(0, 30);
                Outstr("Press 0 to");
                setCursor(0, 40);
                Outstr("read the rules");
                break;

            }
        }
    }

    fillScreen(BLACK);
    drawGrid(GRID_START_X, GRID_START_Y);
    labelGrid(GRID_START_X, GRID_START_Y);

    // Draw and label the second grid (bottom-right)
    drawGrid(GRID2_START_X, GRID2_START_Y);
    labelGrid(GRID2_START_X, GRID2_START_Y);
    setCursor(0,70);
    Outstr("Type in");
    setCursor(0,80);
    Outstr("your");
    setCursor(0,90);
    Outstr("username");

    while(user_name) {

        if (CHECK_FLAG){
            //MAP_GPIOIntDisable(IR.port, IR.pin);
         //   OFF = 1;
            for (i = 0; i < 32; i++) // or with 0 and put into an unsigned long to be checked against the signals from TV remote
            {
                receivedData = (receivedData << 1) | SIGNAL[i+1];
                SIGNAL[i+1] = 0;
            }
               // receivedData = receivedData << 1;

            switch (receivedData){ // check TV remote signal against what is received
            case ZERO:
                message[message_int] = ' ';
                break;
            case ONE:
                if (lastPressed == TWO) {
                    ABC();
                }
                else if (lastPressed == THREE) {
                    DEF();
                }
                else if (lastPressed == FOUR) {
                    GHI();
                }
                else if (lastPressed == FIVE) {
                    JKL();
                }
                else if (lastPressed == SIX) {
                    MNO();
                }
                else if (lastPressed == SEVEN) {
                    PQRS();
                }
                else if (lastPressed == EIGHT) {
                    TUV();
                }
                else if (lastPressed == NINE) {
                    WXYZ();
                }
                break;
            case TWO:
                message[message_int] = 'A';
                lastPressed = TWO;
                Message("A");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                abc = 0;
                break;
            case THREE:
                message[message_int] = 'D';
                lastPressed = THREE;
                Message("D");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                def = 0;
                break;
            case FOUR:
                message[message_int] = 'G';
                lastPressed = FOUR;
                Message("G");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                ghi = 0;
                break;
            case FIVE:
                message[message_int] = 'J';
                lastPressed = FIVE;
                Message("J");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                jkl = 0;
                break;
            case SIX:
                message[message_int] = 'M';
                lastPressed = SIX;
                Message("M");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                mno = 0;
                break;
            case SEVEN:
                message[message_int] = 'P';
                lastPressed = SEVEN;
                Message("P");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                pqrs = 0;
                break;
            case EIGHT:
                message[message_int] = 'T';
                lastPressed = EIGHT;
                Message("T");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                tuv = 0;
                break;
            case NINE:
                message[message_int] = 'W';
                lastPressed = NINE;
                Message("W");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                wxyz = 0;
                break;
            case LAST:
                message[message_int - 1] = ' ';
                message_int -= 2;
                Message("\b \b");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                break;
            case MUTE:
                for (i = 0; i < message_int; i++) {
                    username[i] = message[i];
                }
                local_username_index = message_int; //TEST
                setCursor(80,0);
                Outstr(message);
                fillRect(0, 70, 59, 65, BLACK);
                fillRect(80, 35, 50, 20, BLACK);
                user_name = 0;
                for (i = 0; i < message_int; i++) { // clear message array
                    message[i] = '\0';
                }
                message_int = 0;
                Message("\n");
                break;
            default:
                message_int--;
                break;
        }
     //   IR_intcount = 0;
    //    OFF = 0;
        START_FLAG = 0; //TEST
        CHECK_FLAG = 0; //TEST
        IR_intcount = 0; //TEST
        receivedData = 0;
        message_int++;




        }

    }
    message_int = 0;
    Message("Type in the coordinates of the head and tail of your length 2 ship. Put it in the format XYXY.");

    setCursor(0,70);
    Outstr("Type in");
    setCursor(0,80);
    Outstr("start and ");
    setCursor(0,90);
    Outstr("end");
    setCursor(0,100);
    Outstr("coordinate");
    setCursor(0,110);
    Outstr("for length");
    setCursor(0,120);
    Outstr("2 ship");
    while(first_ship) {
        if (CHECK_FLAG){
            //MAP_GPIOIntDisable(IR.port, IR.pin);
         //   OFF = 1;
            for (i = 0; i < 32; i++) // or with 0 and put into an unsigned long to be checked against the signals from TV remote
            {
                receivedData = (receivedData << 1) | SIGNAL[i+1];
                SIGNAL[i+1] = 0;
            }
               // receivedData = receivedData << 1;
             //   message_int = 0;
            switch (receivedData){ // check TV remote signal against what is received
            case ZERO:
                message[message_int] = ' ';
                break;
            case ONE:
                if (lastPressed == TWO) {
                    ABC();
                }
                else if (lastPressed == THREE) {
                    DEF();
                }
                else if (lastPressed == FOUR) {
                    GHI();
                }
                else if (lastPressed == FIVE) {
                    JKL();
                }
                else if (lastPressed == SIX) {
                    MNO();
                }
                else if (lastPressed == SEVEN) {
                    PQRS();
                }
                else if (lastPressed == EIGHT) {
                    TUV();
                }
                else if (lastPressed == NINE) {
                    WXYZ();
                }
                break;
            case TWO:
                message[message_int] = 'A';
                lastPressed = TWO;
                Message("A");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                abc = 0;
                break;
            case THREE:
                message[message_int] = 'D';
                lastPressed = THREE;
                Message("D");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                def = 0;
                break;
            case FOUR:
                message[message_int] = 'G';
                lastPressed = FOUR;
                Message("G");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                ghi = 0;
                break;
            case FIVE:
                message[message_int] = 'J';
                lastPressed = FIVE;
                Message("J");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                jkl = 0;
                break;
            case SIX:
                message[message_int] = 'M';
                lastPressed = SIX;
                Message("M");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                mno = 0;
                break;
            case SEVEN:
                message[message_int] = 'P';
                lastPressed = SEVEN;
                Message("P");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                pqrs = 0;
                break;
            case EIGHT:
                message[message_int] = 'T';
                lastPressed = EIGHT;
                Message("T");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                tuv = 0;
                break;
            case NINE:
                message[message_int] = 'W';
                lastPressed = NINE;
                Message("W");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                wxyz = 0;
                break;
            case LAST:
                message[message_int - 1] = ' ';
                message_int -= 2;
                Message("\b \b");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                break;
            case MUTE:
                x_coor1_2ship = message[0] - 'A' + 1;
                y_coor1_2ship = message[1] - 'A' + 1;
                x_coor2_2ship = message[2] - 'A' + 1;
                y_coor2_2ship = message[3] - 'A' + 1;

                //TEST
                Report("\n x1: %d, x2: %d, y1: %d, y2: %d, abs(x1-x2): %d, abs(y1-y2):%d  \n", x_coor1_2ship, x_coor2_2ship, y_coor1_2ship, y_coor2_2ship,abs(x_coor2_2ship - x_coor1_2ship), abs(y_coor2_2ship - y_coor1_2ship));
                if (((y_coor1_2ship == y_coor2_2ship) && (abs(x_coor2_2ship - x_coor1_2ship) != 1))
                        || ((x_coor1_2ship == x_coor2_2ship) && (abs(y_coor2_2ship - y_coor1_2ship) != 1))
                        ||((y_coor1_2ship != y_coor2_2ship) && (x_coor1_2ship != x_coor2_2ship))
                        || ((x_coor1_2ship == x_coor2_2ship) && (y_coor2_2ship < y_coor1_2ship))
                        || ((y_coor1_2ship == y_coor2_2ship) && (x_coor2_2ship < x_coor1_2ship))
                        || (message_int!=4)
                        || ((x_coor1_2ship > 7) || (x_coor2_2ship > 7) || (y_coor1_2ship > 7) || (y_coor2_2ship > 7))) {
                    fillRect(0, 70, 59, 65, BLACK);
                    fillRect(80, 35, 50, 20, BLACK);
                    Message("Invalid input try again\n");
                    setCursor(0,70);
                    Outstr("Invalid");
                    setCursor(0,80);
                    Outstr("input");
                    setCursor(0,90);
                    Outstr("Try again ");
                    setCursor(0,100);
                    Outstr("for length");
                    setCursor(0,110);
                    Outstr("2 ship");
                    for (i = 0; i < message_int; i++) {
                                       message[i] = '\0';
                    }
                    message_int = -1;
                    x_coor1_2ship = 0;
                    y_coor1_2ship = 0;
                    x_coor2_2ship = 0;
                    y_coor2_2ship = 0;
                    break;
                }

                    //TEST end
                fillRect(12 + (x_coor1_2ship -1) * 8, 12 + (y_coor1_2ship -1) * 8, 5, 5, BLUE);
                fillRect(12 + (x_coor2_2ship -1) * 8, 12 + (y_coor2_2ship -1) * 8, 5, 5, BLUE);
                ship_locations[0][0] = message[0];
                ship_locations[0][1] = message[1];
                ship_locations[1][0] = message[2];
                ship_locations[1][1] = message[3];
                for (i = 0; i < message_int; i++) {
                    message[i] = '\0';
                }
                fillRect(0, 70, 59, 65, BLACK);
                fillRect(80, 35, 50, 20, BLACK);
                first_ship = 0;
                message_int = 0;
                Message("\n");
                break;
            default:
                message_int--;
                break;
        }
     //   IR_intcount = 0;
    //    OFF = 0;
        START_FLAG = 0; //TEST
        CHECK_FLAG = 0; //TEST
        IR_intcount = 0; //TEST
        receivedData = 0;
        message_int++;




        }

    }
    message_int = 0;
    Message("Type in the coordinates of the head and tail of your length 3 ship. Put it in the format XYXY.");

    setCursor(0,70);
    Outstr("Type in");
    setCursor(0,80);
    Outstr("start and ");
    setCursor(0,90);
    Outstr("end");
    setCursor(0,100);
    Outstr("coordinate");
    setCursor(0,110);
    Outstr("for length");
    setCursor(0,120);
    Outstr("3 ship");
    while(second_ship) {
        if (CHECK_FLAG){
            //MAP_GPIOIntDisable(IR.port, IR.pin);
         //   OFF = 1;
            for (i = 0; i < 32; i++) // or with 0 and put into an unsigned long to be checked against the signals from TV remote
            {
                receivedData = (receivedData << 1) | SIGNAL[i+1];
                SIGNAL[i+1] = 0;
            }
               // receivedData = receivedData << 1;

            switch (receivedData){ // check TV remote signal against what is received
            case ZERO:
                message[message_int] = ' ';
                break;
            case ONE:
                if (lastPressed == TWO) {
                    ABC();
                }
                else if (lastPressed == THREE) {
                    DEF();
                }
                else if (lastPressed == FOUR) {
                    GHI();
                }
                else if (lastPressed == FIVE) {
                    JKL();
                }
                else if (lastPressed == SIX) {
                    MNO();
                }
                else if (lastPressed == SEVEN) {
                    PQRS();
                }
                else if (lastPressed == EIGHT) {
                    TUV();
                }
                else if (lastPressed == NINE) {
                    WXYZ();
                }
                break;
            case TWO:
                message[message_int] = 'A';
                lastPressed = TWO;
                Message("A");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                abc = 0;
                break;
            case THREE:
                message[message_int] = 'D';
                lastPressed = THREE;
                Message("D");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                def = 0;
                break;
            case FOUR:
                message[message_int] = 'G';
                lastPressed = FOUR;
                Message("G");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                ghi = 0;
                break;
            case FIVE:
                message[message_int] = 'J';
                lastPressed = FIVE;
                Message("J");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                jkl = 0;
                break;
            case SIX:
                message[message_int] = 'M';
                lastPressed = SIX;
                Message("M");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                mno = 0;
                break;
            case SEVEN:
                message[message_int] = 'P';
                lastPressed = SEVEN;
                Message("P");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                pqrs = 0;
                break;
            case EIGHT:
                message[message_int] = 'T';
                lastPressed = EIGHT;
                Message("T");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                tuv = 0;
                break;
            case NINE:
                message[message_int] = 'W';
                lastPressed = NINE;
                Message("W");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                wxyz = 0;
                break;
            case LAST:
                message[message_int - 1] = ' ';
                message_int -= 2;
                Message("\b \b");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                break;
            case MUTE:
                x_coor1_3ship = message[0] - 'A' + 1;
                y_coor1_3ship = message[1] - 'A' + 1;
                x_coor2_3ship = message[2] - 'A' + 1;
                y_coor2_3ship = message[3] - 'A' + 1;

                //Error checking
                Report("\n x1: %d, x2: %d, y1: %d, y2: %d, abs(x1-x2): %d, abs(y1-y2):%d  \n", x_coor1_3ship, x_coor2_3ship, y_coor1_3ship, y_coor2_3ship,abs(x_coor2_3ship - x_coor1_3ship), abs(y_coor2_3ship - y_coor1_3ship));
                                if (((y_coor1_3ship == y_coor2_3ship) && (abs(x_coor2_3ship - x_coor1_3ship) != 2))
                                        || ((x_coor1_3ship == x_coor2_3ship) && (abs(y_coor2_3ship - y_coor1_3ship) != 2))
                                        ||((y_coor1_3ship != y_coor2_3ship) && (x_coor1_3ship != x_coor2_3ship))
                                        || (message_int!=4)
                                        || ((x_coor1_3ship == x_coor2_3ship) && (y_coor2_3ship < y_coor1_3ship))
                                        || ((y_coor1_3ship == y_coor2_3ship) && (x_coor2_3ship < x_coor1_3ship))
                                        || ((x_coor1_3ship > 7) || (x_coor2_3ship > 7) || (y_coor1_3ship > 7) || (y_coor2_3ship > 7))) {
                                    fillRect(0, 70, 59, 65, BLACK);
                                    fillRect(80, 35, 50, 20, BLACK);
                                    Message("Invalid input try again\n");
                                    setCursor(0,70);
                                    Outstr("Invalid");
                                    setCursor(0,80);
                                    Outstr("input");
                                    setCursor(0,90);
                                    Outstr("Try again ");
                                    setCursor(0,100);
                                    Outstr("for length");
                                    setCursor(0,110);
                                    Outstr("3 ship");
                                    for (i = 0; i < message_int; i++) {
                                                       message[i] = '\0';
                                    }
                                    message_int = -1;
                                    x_coor1_2ship = 0;
                                    y_coor1_2ship = 0;
                                    x_coor2_2ship = 0;
                                    y_coor2_2ship = 0;
                                    break;
                                }
                // Error checking end

                   //overlap checking

                if (x_coor1_3ship == x_coor2_3ship) {
                    if (((message[0] == ship_locations[0][0]) && (message[1] == ship_locations[0][1]))
                        ||((message[0] == ship_locations[1][0]) && (message[1] == ship_locations[1][1]))
                        ||((message[0] == ship_locations[0][0]) && ((message[1]+1) == ship_locations[0][1]))
                        ||((message[0] == ship_locations[1][0]) && ((message[1]+1) == ship_locations[1][1]))
                        ||((message[2] == ship_locations[0][0]) && ((message[3]) == ship_locations[0][1]))
                        ||((message[2] == ship_locations[1][0]) && ((message[3]) == ship_locations[1][1]))){

                        Message("ship2 overlap with ship 1");
                        fillRect(0, 70, 59, 65, BLACK);
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(0,70);
                        Outstr("Overlapped");
                        setCursor(0,80);
                        Outstr("with ship1");
                        setCursor(0,90);
                        Outstr("Try again ");
                        setCursor(0,100);
                        Outstr("for length");
                        setCursor(0,110);
                        Outstr("3 ship");
                        for (i = 0; i < message_int; i++) {
                                           message[i] = '\0';
                        }
                        message_int = -1;
                        x_coor1_3ship = 0;
                        y_coor1_3ship = 0;
                        x_coor2_3ship = 0;
                        y_coor2_3ship = 0;
                        break;
                    }
                }


                if (y_coor1_3ship == y_coor2_3ship) {
                    if (((message[0] == ship_locations[0][0]) && (message[1] == ship_locations[0][1]))
                        ||((message[0] == ship_locations[1][0]) && (message[1] == ship_locations[1][1]))
                        ||((message[0] + 1 == ship_locations[0][0]) && ((message[1]) == ship_locations[0][1]))
                        ||((message[0] + 1 == ship_locations[1][0]) && ((message[1]) == ship_locations[1][1]))
                        ||((message[2] == ship_locations[0][0]) && ((message[3]) == ship_locations[0][1]))
                        ||((message[2] == ship_locations[1][0]) && ((message[3]) == ship_locations[1][1]))){

                        Message("ship2 overlap with ship 1");
                        fillRect(0, 70, 59, 65, BLACK);
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(0,70);
                        Outstr("Overlapped");
                        setCursor(0,80);
                        Outstr("with ship1");
                        setCursor(0,90);
                        Outstr("Try again ");
                        setCursor(0,100);
                        Outstr("for length");
                        setCursor(0,110);
                        Outstr("3 ship");
                        for (i = 0; i < message_int; i++) {
                                           message[i] = '\0';
                        }
                        message_int = -1;
                        x_coor1_3ship = 0;
                        y_coor1_3ship = 0;
                        x_coor2_3ship = 0;
                        y_coor2_3ship = 0;
                        break;
                    }
                }
                //end overlap

                if (x_coor1_3ship == x_coor2_3ship) {
                    for (i = 0; i <= (y_coor2_3ship - y_coor1_3ship); i++) {
                        fillRect(12 + (x_coor1_3ship -1) * 8, 12 + (y_coor1_3ship + i -1) * 8, 5, 5, BLUE);
                    }
                    ship_locations[2][0] = message[0];
                    ship_locations[2][1] = message[1];
                    ship_locations[3][0] = message[0];
                    ship_locations[3][1] = message[1] + 1;
                    ship_locations[4][0] = message[2];
                    ship_locations[4][1] = message[3];

                }
                else if (y_coor1_3ship == y_coor2_3ship) {
                    for (i = 0; i <= (x_coor2_3ship - x_coor1_3ship); i++) {
                        fillRect(12 + (x_coor1_3ship + i -1) * 8, 12 + (y_coor1_3ship - 1) * 8, 5, 5, BLUE);
                    }
                    ship_locations[2][0] = message[0];
                    ship_locations[2][1] = message[1];
                    ship_locations[3][0] = message[0] + 1;
                    ship_locations[3][1] = message[1];
                    ship_locations[4][0] = message[2];
                    ship_locations[4][1] = message[3];
                }
                for (i = 0; i < message_int; i++) {
                    message[i] = '\0';
                }
                fillRect(0, 70, 59, 65, BLACK);
                fillRect(80, 35, 50, 20, BLACK);
                second_ship = 0;
                message_int = 0;
                Message("\n");
                break;
            default:
                message_int--;
                break;
        }
     //   IR_intcount = 0;
    //    OFF = 0;
        START_FLAG = 0; //TEST
        CHECK_FLAG = 0; //TEST
        IR_intcount = 0; //TEST
        receivedData = 0;
        message_int++;




        }

    }
    message_int = 0;
    Message("Type in the coordinates of the head and tail of your length 4 ship. Put it in the format XYXY.");

    setCursor(0,70);
    Outstr("Type in");
    setCursor(0,80);
    Outstr("start and ");
    setCursor(0,90);
    Outstr("end");
    setCursor(0,100);
    Outstr("coordinate");
    setCursor(0,110);
    Outstr("for length");
    setCursor(0,120);
    Outstr("4 ship");
    while(third_ship) {
        if (CHECK_FLAG){
            //MAP_GPIOIntDisable(IR.port, IR.pin);
         //   OFF = 1;
            for (i = 0; i < 32; i++) // or with 0 and put into an unsigned long to be checked against the signals from TV remote
            {
                receivedData = (receivedData << 1) | SIGNAL[i+1];
                SIGNAL[i+1] = 0;
            }
               // receivedData = receivedData << 1;

            switch (receivedData){ // check TV remote signal against what is received
            case ZERO:
                message[message_int] = ' ';
                break;
            case ONE:
                if (lastPressed == TWO) {
                    ABC();
                }
                else if (lastPressed == THREE) {
                    DEF();
                }
                else if (lastPressed == FOUR) {
                    GHI();
                }
                else if (lastPressed == FIVE) {
                    JKL();
                }
                else if (lastPressed == SIX) {
                    MNO();
                }
                else if (lastPressed == SEVEN) {
                    PQRS();
                }
                else if (lastPressed == EIGHT) {
                    TUV();
                }
                else if (lastPressed == NINE) {
                    WXYZ();
                }
                break;
            case TWO:
                message[message_int] = 'A';
                lastPressed = TWO;
                Message("A");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                abc = 0;
                break;
            case THREE:
                message[message_int] = 'D';
                lastPressed = THREE;
                Message("D");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                def = 0;
                break;
            case FOUR:
                message[message_int] = 'G';
                lastPressed = FOUR;
                Message("G");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                ghi = 0;
                break;
            case FIVE:
                message[message_int] = 'J';
                lastPressed = FIVE;
                Message("J");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                jkl = 0;
                break;
            case SIX:
                message[message_int] = 'M';
                lastPressed = SIX;
                Message("M");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                mno = 0;
                break;
            case SEVEN:
                message[message_int] = 'P';
                lastPressed = SEVEN;
                Message("P");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                pqrs = 0;
                break;
            case EIGHT:
                message[message_int] = 'T';
                lastPressed = EIGHT;
                Message("T");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                tuv = 0;
                break;
            case NINE:
                message[message_int] = 'W';
                lastPressed = NINE;
                Message("W");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                wxyz = 0;
                break;
            case LAST:
                message[message_int - 1] = ' ';
                message_int -= 2;
                Message("\b \b");
                fillRect(80, 35, 50, 20, BLACK);
                setCursor(80, 35);
                Outstr(message);
                break;
            case MUTE:
                x_coor1_4ship = message[0] - 'A' + 1;
                y_coor1_4ship = message[1] - 'A' + 1;
                x_coor2_4ship = message[2] - 'A' + 1;
                y_coor2_4ship = message[3] - 'A' + 1;

                //error checking
                Report("\n x1: %d, x2: %d, y1: %d, y2: %d, abs(x1-x2): %d, abs(y1-y2):%d  \n", x_coor1_4ship, x_coor2_4ship, y_coor1_4ship, y_coor2_4ship,abs(x_coor2_4ship - x_coor1_4ship), abs(y_coor2_4ship - y_coor1_4ship));
                                if (((y_coor1_4ship == y_coor2_4ship) && (abs(x_coor2_4ship - x_coor1_4ship) != 3))
                                        || ((x_coor1_4ship == x_coor2_4ship) && (abs(y_coor2_4ship - y_coor1_4ship) != 3))
                                        ||((y_coor1_4ship != y_coor2_4ship) && (x_coor1_4ship != x_coor2_4ship))
                                        || (message_int!=4)
                                        || ((x_coor1_4ship == x_coor2_4ship) && (y_coor2_4ship < y_coor1_4ship))
                                        || ((y_coor1_4ship == y_coor2_4ship) && (x_coor2_4ship < x_coor1_4ship))
                                        || ((x_coor1_4ship > 7) || (x_coor2_4ship > 7) || (y_coor1_4ship > 7) || (y_coor2_4ship > 7))) {
                                    fillRect(0, 70, 59, 65, BLACK);
                                    fillRect(80, 35, 50, 20, BLACK);
                                    Message("Invalid input try again\n");
                                    setCursor(0,70);
                                    Outstr("Invalid");
                                    setCursor(0,80);
                                    Outstr("input");
                                    setCursor(0,90);
                                    Outstr("Try again ");
                                    setCursor(0,100);
                                    Outstr("for length");
                                    setCursor(0,110);
                                    Outstr("4 ship");
                                    for (i = 0; i < message_int; i++) {
                                                       message[i] = '\0';
                                    }
                                    message_int = -1;
                                    x_coor1_2ship = 0;
                                    y_coor1_2ship = 0;
                                    x_coor2_2ship = 0;
                                    y_coor2_2ship = 0;
                                    break;
                                }

                                //error checking end

                                //overlap checking
                                //overlap with ship1
                                if (x_coor1_4ship == x_coor2_4ship) {
                                if (((message[0] == ship_locations[0][0]) && (message[1] == ship_locations[0][1]))
                                    ||((message[0] == ship_locations[1][0]) && (message[1] == ship_locations[1][1]))
                                    ||((message[0] == ship_locations[0][0]) && ((message[1]+1) == ship_locations[0][1]))
                                    ||((message[0] == ship_locations[1][0]) && ((message[1]+1) == ship_locations[1][1]))
                                    ||((message[0] == ship_locations[0][0]) && ((message[1]+2) == ship_locations[0][1]))
                                    ||((message[0] == ship_locations[1][0]) && ((message[1]+2) == ship_locations[1][1]))
                                    ||((message[2] == ship_locations[0][0]) && ((message[3]) == ship_locations[0][1]))
                                    ||((message[2] == ship_locations[1][0]) && ((message[3]) == ship_locations[1][1]))){

                                    Message("ship3 overlap with ship1");
                                    fillRect(0, 70, 59, 65, BLACK);
                                    fillRect(80, 35, 50, 20, BLACK);
                                    setCursor(0,70);
                                    Outstr("Overlapped");
                                    setCursor(0,80);
                                    Outstr("with ship1");
                                    setCursor(0,90);
                                    Outstr("Try again ");
                                    setCursor(0,100);
                                    Outstr("for length");
                                    setCursor(0,110);
                                    Outstr("4 ship");
                                    for (i = 0; i < message_int; i++) {
                                                       message[i] = '\0';
                                    }
                                    message_int = -1;
                                    x_coor1_4ship = 0;
                                    y_coor1_4ship = 0;
                                    x_coor2_4ship = 0;
                                    y_coor2_4ship = 0;
                                    break;
                                }
                            }


                            if (y_coor1_4ship == y_coor2_4ship) {
                                if (((message[0] == ship_locations[0][0]) && (message[1] == ship_locations[0][1]))
                                    ||((message[0] == ship_locations[1][0]) && (message[1] == ship_locations[1][1]))
                                    ||((message[0] + 1 == ship_locations[0][0]) && ((message[1]) == ship_locations[0][1]))
                                    ||((message[0] + 1 == ship_locations[1][0]) && ((message[1]) == ship_locations[1][1]))
                                    ||((message[0] + 2 == ship_locations[0][0]) && ((message[1]) == ship_locations[0][1]))
                                    ||((message[0] + 2== ship_locations[1][0]) && ((message[1]) == ship_locations[1][1]))
                                    ||((message[2] == ship_locations[0][0]) && ((message[3]) == ship_locations[0][1]))
                                    ||((message[2] == ship_locations[1][0]) && ((message[3]) == ship_locations[1][1]))){

                                    Message("ship3 overlap with ship1");
                                    fillRect(0, 70, 59, 65, BLACK);
                                    fillRect(80, 35, 50, 20, BLACK);
                                    setCursor(0,70);
                                    Outstr("Overlapped");
                                    setCursor(0,80);
                                    Outstr("with ship1");
                                    setCursor(0,90);
                                    Outstr("Try again ");
                                    setCursor(0,100);
                                    Outstr("for length");
                                    setCursor(0,110);
                                    Outstr("4 ship");
                                    for (i = 0; i < message_int; i++) {
                                                       message[i] = '\0';
                                    }
                                    message_int = -1;
                                    x_coor1_4ship = 0;
                                    y_coor1_4ship = 0;
                                    x_coor2_4ship = 0;
                                    y_coor2_4ship = 0;

                                    break;
                                }
                            }

                            //overlap with ship2
                            if (x_coor1_4ship == x_coor2_4ship) {
                                if (((message[0] == ship_locations[2][0]) && (message[1] == ship_locations[2][1]))
                                    ||((message[0] == ship_locations[3][0]) && (message[1] == ship_locations[3][1]))
                                    ||((message[0] == ship_locations[4][0]) && (message[1] == ship_locations[4][1]))
                                    ||((message[0] == ship_locations[2][0]) && ((message[1]+1) == ship_locations[2][1]))
                                    ||((message[0] == ship_locations[3][0]) && ((message[1]+1) == ship_locations[3][1]))
                                    ||((message[0] == ship_locations[4][0]) && ((message[1]+1) == ship_locations[4][1]))
                                    ||((message[0] == ship_locations[2][0]) && ((message[1]+2) == ship_locations[2][1]))
                                    ||((message[0] == ship_locations[3][0]) && ((message[1]+2) == ship_locations[3][1]))
                                    ||((message[0] == ship_locations[4][0]) && ((message[1]+2) == ship_locations[4][1]))
                                    ||((message[2] == ship_locations[2][0]) && ((message[3]) == ship_locations[2][1]))
                                    ||((message[2] == ship_locations[3][0]) && ((message[3]) == ship_locations[3][1]))
                                    ||((message[2] == ship_locations[4][0]) && ((message[3]) == ship_locations[4][1]))){

                                    Message("ship3 overlap with ship2");
                                    fillRect(0, 70, 59, 65, BLACK);
                                    fillRect(80, 35, 50, 20, BLACK);
                                    setCursor(0,70);
                                    Outstr("Overlapped");
                                    setCursor(0,80);
                                    Outstr("with ship2");
                                    setCursor(0,90);
                                    Outstr("Try again ");
                                    setCursor(0,100);
                                    Outstr("for length");
                                    setCursor(0,110);
                                    Outstr("4 ship");
                                    for (i = 0; i < message_int; i++) {
                                                       message[i] = '\0';
                                    }
                                    message_int = -1;
                                    x_coor1_4ship = 0;
                                    y_coor1_4ship = 0;
                                    x_coor2_4ship = 0;
                                    y_coor2_4ship = 0;
                                    break;
                                }
                            }
                            if (y_coor1_4ship == y_coor2_4ship) {
                                if (((message[0] == ship_locations[2][0]) && (message[1] == ship_locations[2][1]))
                                    ||((message[0] == ship_locations[3][0]) && (message[1] == ship_locations[3][1]))
                                    ||((message[0] == ship_locations[4][0]) && (message[1] == ship_locations[4][1]))
                                    ||((message[0]+1 == ship_locations[2][0]) && ((message[1]) == ship_locations[2][1]))
                                    ||((message[0]+1 == ship_locations[3][0]) && ((message[1]) == ship_locations[3][1]))
                                    ||((message[0]+1 == ship_locations[4][0]) && ((message[1]) == ship_locations[4][1]))
                                    ||((message[0]+2 == ship_locations[2][0]) && ((message[1]) == ship_locations[2][1]))
                                    ||((message[0]+2 == ship_locations[3][0]) && ((message[1]) == ship_locations[3][1]))
                                    ||((message[0]+2 == ship_locations[4][0]) && ((message[1]) == ship_locations[4][1]))
                                    ||((message[2] == ship_locations[2][0]) && ((message[3]) == ship_locations[2][1]))
                                    ||((message[2] == ship_locations[3][0]) && ((message[3]) == ship_locations[3][1]))
                                    ||((message[2] == ship_locations[4][0]) && ((message[3]) == ship_locations[4][1]))){

                                    Message("ship3 overlap with ship2");
                                    fillRect(0, 70, 59, 65, BLACK);
                                    fillRect(80, 35, 50, 20, BLACK);
                                    setCursor(0,70);
                                    Outstr("Overlapped");
                                    setCursor(0,80);
                                    Outstr("with ship2");
                                    setCursor(0,90);
                                    Outstr("Try again ");
                                    setCursor(0,100);
                                    Outstr("for length");
                                    setCursor(0,110);
                                    Outstr("4 ship");
                                    for (i = 0; i < message_int; i++) {
                                                       message[i] = '\0';
                                    }
                                    message_int = -1;
                                    x_coor1_4ship = 0;
                                    y_coor1_4ship = 0;
                                    x_coor2_4ship = 0;
                                    y_coor2_4ship = 0;
                                    break;
                                }
                            }

                               //overlap checking end

                if (x_coor1_4ship == x_coor2_4ship) {
                    for (i = 0; i <= (y_coor2_4ship - y_coor1_4ship); i++) {
                        fillRect(12 + (x_coor1_4ship -1) * 8, 12 + (y_coor1_4ship + i -1) * 8, 5, 5, BLUE);
                    }
                    ship_locations[5][0] = message[0];
                    ship_locations[5][1] = message[1];
                    ship_locations[6][0] = message[0];
                    ship_locations[6][1] = message[1] + 1;
                    ship_locations[7][0] = message[0];
                    ship_locations[7][1] = message[1] + 2;
                    ship_locations[8][0] = message[2];
                    ship_locations[8][1] = message[3];
                }
                else if (y_coor1_4ship == y_coor2_4ship) {
                    for (i = 0; i <= (x_coor2_4ship - x_coor1_4ship); i++) {
                        fillRect(12 + (x_coor1_4ship + i -1) * 8, 12 + (y_coor1_4ship - 1) * 8, 5, 5, BLUE);
                    }
                    ship_locations[5][0] = message[0];
                    ship_locations[5][1] = message[1];
                    ship_locations[6][0] = message[0] + 1;
                    ship_locations[6][1] = message[1];
                    ship_locations[7][0] = message[0] + 2;
                    ship_locations[7][1] = message[1];
                    ship_locations[8][0] = message[2];
                    ship_locations[8][1] = message[3];
                }
                for (i = 0; i < message_int; i++) {
                    message[i] = '\0';
                }
                fillRect(0, 70, 59, 65, BLACK);
                fillRect(80, 35, 50, 20, BLACK);
                third_ship = 0;
                message_int = 0;
                Message("\n");
                break;
            default:
                message_int--;
                break;
        }
     //   IR_intcount = 0;
    //    OFF = 0;
        START_FLAG = 0; //TEST
        CHECK_FLAG = 0; //TEST
        IR_intcount = 0; //TEST
        receivedData = 0;
        message_int++;




        }

    }
    message_int = 0;
    int game_not_over = 1;//TEST
    setCursor(0,70);
    Outstr("Type in a");
    setCursor(0,80);
    Outstr("coordinate");
    setCursor(0,90);
    Outstr("you would");
    setCursor(0,100);
    Outstr("like to");
    setCursor(0,110);
    Outstr("shoot");
    while(game_not_over) {

       // if(IR_intflag == 0)
        //Report("\t\t\t FLAG IS OFF \n\r");
        //if(IR_intflag == 1)
       // Report("\t\t\t FLAG IS ON \n\r");
       // while(IR_intflag == 0){
           // Report("\t\t\t FLAG OFF \n\r");
       // ;}
        //}
       // if (IR_intflag){
          //  IR_intflag = 0;
          //  Report("\t\t\t #####################################################REACH HERE \n\r");
      //  }
        if (CHECK_FLAG){
                    //MAP_GPIOIntDisable(IR.port, IR.pin);
                 //   OFF = 1;
                    for (i = 0; i < 32; i++) // or with 0 and put into an unsigned long to be checked against the signals from TV remote
                    {
                        receivedData = (receivedData << 1) | SIGNAL[i+1];
                        SIGNAL[i+1] = 0;
                    }
                       // receivedData = receivedData << 1;

                    switch (receivedData){ // check TV remote signal against what is received
                    case ZERO:
                        message[message_int] = ' ';
                        break;
                    case ONE:
                        if (lastPressed == TWO) {
                            ABC();
                        }
                        else if (lastPressed == THREE) {
                            DEF();
                        }
                        else if (lastPressed == FOUR) {
                            GHI();
                        }
                        else if (lastPressed == FIVE) {
                            JKL();
                        }
                        else if (lastPressed == SIX) {
                            MNO();
                        }
                        else if (lastPressed == SEVEN) {
                            PQRS();
                        }
                        else if (lastPressed == EIGHT) {
                            TUV();
                        }
                        else if (lastPressed == NINE) {
                            WXYZ();
                        }
                        break;
                    case TWO:
                        message[message_int] = 'A';
                        lastPressed = TWO;
                        Message("A");
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(80, 35);
                        Outstr(message);
                        abc = 0;
                        break;
                    case THREE:
                        message[message_int] = 'D';
                        lastPressed = THREE;
                        Message("D");
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(80, 35);
                        Outstr(message);
                        def = 0;
                        break;
                    case FOUR:
                        message[message_int] = 'G';
                        lastPressed = FOUR;
                        Message("G");
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(80, 35);
                        Outstr(message);
                        ghi = 0;
                        break;
                    case FIVE:
                        message[message_int] = 'J';
                        lastPressed = FIVE;
                        Message("J");
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(80, 35);
                        Outstr(message);
                        jkl = 0;
                        break;
                    case SIX:
                        message[message_int] = 'M';
                        lastPressed = SIX;
                        Message("M");
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(80, 35);
                        Outstr(message);
                        mno = 0;
                        break;
                    case SEVEN:
                        message[message_int] = 'P';
                        lastPressed = SEVEN;
                        Message("P");
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(80, 35);
                        Outstr(message);
                        pqrs = 0;
                        break;
                    case EIGHT:
                        message[message_int] = 'T';
                        lastPressed = EIGHT;
                        Message("T");
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(80, 35);
                        Outstr(message);
                        tuv = 0;
                        break;
                    case NINE:
                        message[message_int] = 'W';
                        lastPressed = NINE;
                        Message("W");
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(80, 35);
                        Outstr(message);
                        wxyz = 0;
                        break;
                    case LAST:
                        message[message_int - 1] = ' ';
                        message_int -= 2;
                        Message("\b \b");
                        fillRect(80, 35, 50, 20, BLACK);
                        setCursor(80, 35);
                        Outstr(message);
                        break;
                    case MUTE:

                       // error checking
                        Report(" \n attack_coordinateX = %d, attack_coordinateY = %d, message_int = %d\n", message[0]-'A' + 1, message[1] - 'A' + 1, message_int);
                        if ((((message[0]-'A' + 1) > 7) ||((message[1] - 'A' + 1) > 7)) || (message_int != 2)){
                            fillRect(0, 70, 59, 65, BLACK);
                            fillRect(80, 35, 50, 20, BLACK);
                            Message("Invalid input try again\n");
                            setCursor(0,70);
                             Outstr("Invalid");
                             setCursor(0,80);
                             Outstr("input");
                             setCursor(0,90);
                             Outstr("Try again ");
                             setCursor(0,100);
                             Outstr("for ");
                             setCursor(0,110);
                             Outstr("shooting");
                             for (i = 0; i < message_int; i++) {
                                  message[i] = '\0';
                             }
                              message_int = -1;

                        break;

                        }
                        for (i = 0; i < message_int; i++) {
                            UARTCharPut(UARTA1_BASE, message[i]);
                        }


                        //error checking end
                        for (i = 0; i < message_int; i++) {
                            attack_coordinate[i] = message[i];
                            message[i] = '\0';
                        }



                        fillRect(0, 70, 59, 65, BLACK);
                        fillRect(80, 35, 50, 20, BLACK);
                        message_int = -1;
                        Message("\n");
                        break;
                    default:
                        message_int--;
                        break;
                }

             //   IR_intcount = 0;
            //    OFF = 0;
                START_FLAG = 0; //TEST
                CHECK_FLAG = 0; //TEST
                IR_intcount = 0; //TEST
                receivedData = 0;
                message_int++;




                }
        if (strcmp(UART_Recv, hit) == 0) {
            counter += 1;
            turn_counter += 1;
            setCursor(0, 70);
            fillRect(0, 70, 50, 50, BLACK);
            Outstr("HIT!");
            for (i = 0; i < recvIndex; i++) {
                UART_Recv[i] = '\0';
            }
            recvIndex = 0;
            x_attack_coor = attack_coordinate[0] - 'A' + 1;
            y_attack_coor = attack_coordinate[1] - 'A' + 1;
            fillRect(72 + (x_attack_coor -1) * 8, 72 + (y_attack_coor -1) * 8, 5, 5, RED);
            x_attack_coor = 0;
            y_attack_coor = 0;
            for (i = 0; i < 2; i++) {
                attack_coordinate[i] = '\0';
            }

            int enemy_turn = 1;
            setCursor(0,80);
            Outstr("Enemy's");
            setCursor(0,90);
            Outstr("Turn");
            while (enemy_turn) {
                if (strlen(UART_Recv) > 0) {
                    fillRect(0, 70, 50, 50, BLACK);
                    setCursor(0,70);
                    Outstr("Type in a");
                    setCursor(0,80);
                    Outstr("coordinate");
                    setCursor(0,90);
                    Outstr("you would");
                    setCursor(0,100);
                    Outstr("like to");
                    setCursor(0,110);
                    Outstr("shoot");
                    enemy_turn = 0;

                }
                if (counter == 9) {
                    game_not_over = 0;
                    fillScreen(BLACK);
                    setCursor(40, 21);
                    Outstr("Game Over!");
                    setCursor(10, 31);
                    Outstr("You have won!");
                    setCursor(10, 41);
                    Outstr("Stats will be sent");
                    setCursor(40, 51);
                    Outstr("to email");
                    setCursor(40, 61);
                    Outstr("-Press SW3");
                    setCursor(20, 71);
                    Outstr("to send result");
                    setCursor(40, 81);
                    Outstr("-Press 1");
                    setCursor(20, 91);
                    Outstr("to receive email");

                    UARTCharPut(UARTA1_BASE, 'X');
                    UARTCharPut(UARTA1_BASE, 'X');
                    UARTCharPut(UARTA1_BASE, 'X');
                    Win = 1;
                    break;
                }

            }

            Turns_to_finish = Turns_to_finish + 1; // score
        }
        else if (strcmp(UART_Recv, miss) == 0) {
            turn_counter += 1;
            setCursor(0, 70);
            fillRect(0, 70, 50, 50, BLACK);
            Outstr("MISS!");
            for (i = 0; i < recvIndex; i++) {
                UART_Recv[i] = '\0';
            }
            recvIndex = 0;
            x_attack_coor = attack_coordinate[0] - 'A' + 1;
            y_attack_coor = attack_coordinate[1] - 'A' + 1;
            fillRect(72 + (x_attack_coor -1) * 8, 72 + (y_attack_coor -1) * 8, 5, 5, GREEN);
            x_attack_coor = 0;
            y_attack_coor = 0;
            for (i = 0; i < 2; i++) {
                attack_coordinate[i] = '\0';
            }

            int enemy_turn = 1;
            setCursor(0,80);
            Outstr("Enemy's");
            setCursor(0,90);
            Outstr("Turn");
            while (enemy_turn) {
                if (strlen(UART_Recv) > 0) {
                    fillRect(0, 70, 50, 50, BLACK);
                    setCursor(0,70);
                    Outstr("Type in a");
                    setCursor(0,80);
                    Outstr("coordinate");
                    setCursor(0,90);
                    Outstr("you would");
                    setCursor(0,100);
                    Outstr("like to");
                    setCursor(0,110);
                    Outstr("shoot");
                    enemy_turn = 0;

                }
            }

        }
        else if (strlen(UART_Recv) == 2){
            int run_miss = 1;
            for (i = 0; i < 9; i++) {
                if (strcmp(UART_Recv, ship_locations[i]) == 0) {
                    UARTCharPut(UARTA1_BASE, 'H');
                    UARTCharPut(UARTA1_BASE, 'I');
                    UARTCharPut(UARTA1_BASE, 'T');
                    x_attack_coor = UART_Recv[0] - 'A' + 1;
                    y_attack_coor = UART_Recv[1] - 'A' + 1;
                    fillRect(12 + (x_attack_coor -1) * 8, 12 + (y_attack_coor -1) * 8, 5, 5, RED);
                    x_attack_coor = 0;
                    y_attack_coor = 0;
                    for (i = 0; i < 2; i++) {
                        attack_coordinate[i] = '\0';
                    }
                    for (i = 0; i < recvIndex; i++) {
                        UART_Recv[i] = '\0';
                    }
                    recvIndex = 0;
                    run_miss = 0;
                    break;
                }
            }






            if (run_miss) {
                UARTCharPut(UARTA1_BASE, 'M');
                UARTCharPut(UARTA1_BASE, 'I');
                UARTCharPut(UARTA1_BASE, 'S');
                UARTCharPut(UARTA1_BASE, 'S');
                x_attack_coor = UART_Recv[0] - 'A' + 1;
                y_attack_coor = UART_Recv[1] - 'A' + 1;
                fillRect(12 + (x_attack_coor -1) * 8, 12 + (y_attack_coor -1) * 8, 5, 5, GREEN);
                x_attack_coor = 0;
                y_attack_coor = 0;
                for (i = 0; i < 2; i++) {
                    attack_coordinate[i] = '\0';
                }
                for (i = 0; i < recvIndex; i++) {
                    UART_Recv[i] = '\0';
                }
                recvIndex = 0;
                Turns_to_finish = Turns_to_finish + 1; //score
            }


        }
        else if(strcmp(UART_Recv, gameover) == 0) {
            game_not_over = 0;
            fillScreen(BLACK);
            setCursor(40, 21);
            Outstr("Game Over!");
            setCursor(10, 31);
            Outstr("Opponent has won!");
            setCursor(10, 41);
            Outstr("Stats will be sent");
            setCursor(40, 51);
            Outstr("to email");
            setCursor(40, 61);
            Outstr("-Press SW3");
            setCursor(20, 71);
            Outstr("to send result");
            setCursor(40, 81);
            Outstr("-Press 1");
            setCursor(20, 91);
            Outstr("to receive email");


            Win = 2;
        }



    }


    for (i = 0; i < recvIndex; i++) {
                        UART_Recv[i] = '\0';
                    }
    recvIndex = 0;

    int SEND_AWS = 1;
    int RECV_AWS = 1;
      while (SEND_AWS){
       if (GPIOPinRead(GPIOA1_BASE, 0x20) == 0x20){
           for (i = 0; i < local_username_index; i++){
                  UARTCharPut(UARTA1_BASE,username[i]);
              }
           sprintf(RoundNumber, "%d", Turns_to_finish);
           if ((Turns_to_finish) >= 10){
           UARTCharPut(UARTA1_BASE,RoundNumber[0]);
           UARTCharPut(UARTA1_BASE,RoundNumber[1]);
           }
           else if ((Turns_to_finish)<10){
               UARTCharPut(UARTA1_BASE,RoundNumber[0]);
           }
           Message("\nName send");
              SEND_AWS = 0;
       }

      }

      while (RECV_AWS){

      if (CHECK_FLAG){
                  //MAP_GPIOIntDisable(IR.port, IR.pin);
               //   OFF = 1;
                  for (i = 0; i < 32; i++) // or with 0 and put into an unsigned long to be checked against the signals from TV remote
                  {
                      receivedData = (receivedData << 1) | SIGNAL[i+1];
                      SIGNAL[i+1] = 0;
                  }
                     // receivedData = receivedData << 1;

                  switch (receivedData){ // check TV remote signal against what is received
                  case ONE:
                      RECV_AWS = 0;
                      break;
                  default:
                      RECV_AWS = 1;
                      break;
              }

      }
      }
      for ( i = 0; i < local_username_index; i ++){
          Remote_username[i] = UART_Recv[i];
      }

     // if ((Turns_to_finish) >= 10){

              round_to_email[0] = UART_Recv[local_username_index];
              round_to_email[1] = UART_Recv[local_username_index+1];
             /* if (!isalpha(round_to_email[1])) {
                  round_to_email[1] = round_to_email[0];
                  round_to_email[0] = '0';
              }*/
              //           }
      /*else if ((Turns_to_finish)<10){
          round_to_email[0] = UART_Recv[local_username_index];
      }
*/

      if (Win == 1)
      Report("Local username: %s, Remote username: %s, Winner: %s, turns: %s\n", username, Remote_username, username, RoundNumber);
      else if (Win == 2)
          Report("Local username: %s, Remote username: %s, Winner: %s, turns: %s\n", username, Remote_username, Remote_username, round_to_email);


    http_post(lRetVal);
    http_get(lRetVal);

    sl_Stop(SL_STOP_TIMEOUT);
    recvIndex = 0;

    LOOP_FOREVER();

}


static int http_post(int iTLSSockID){
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal = 0;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, POSTHEADER);
    pcBufHeaders += strlen(POSTHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

    char DATA1[512];
    char* databuf = DATA1;

    strcpy(databuf, DATASTART);
           databuf += strlen(DATASTART);

           strcpy(databuf, MESSAGESTART);
           databuf += strlen(MESSAGESTART);

           strcpy(databuf, Local_player);
           databuf += strlen(Local_player);



           strcpy(databuf, username);
           databuf += strlen(username);

           strcpy(databuf, space);
           databuf += strlen(space);

           strcpy(databuf, Remote_player);
           databuf += strlen(Remote_player);

           strcpy(databuf, Remote_username);
           databuf += strlen(Remote_username);

           strcpy(databuf, winnerstatement);
           databuf += strlen(winnerstatement);

          if (Win == 1) {
           strcpy(databuf, username);
           databuf += strlen(username);
           }
           else if (Win == 2){
            strcpy(databuf, Remote_username);
           databuf += strlen(Remote_username);
           }

           strcpy(databuf, round);
           databuf += strlen(round);

           if (Win == 1){
           strcpy(databuf, RoundNumber);
           databuf += strlen(RoundNumber);
           }
           else if (Win == 2){
            strcpy(databuf, round_to_email);
           databuf += strlen(round_to_email);

           }



           strcpy(databuf, MESSAGEMID);
           databuf += strlen(MESSAGEMID);
           strcpy(databuf, Local_player);
           databuf += strlen(Local_player);



           strcpy(databuf, username);
           databuf += strlen(username);

           strcpy(databuf, space);
           databuf += strlen(space);

           strcpy(databuf, Remote_player);
           databuf += strlen(Remote_player);

           strcpy(databuf, Remote_username);
           databuf += strlen(Remote_username);

           strcpy(databuf, winnerstatement);
           databuf += strlen(winnerstatement);

           if (Win == 1) {
           strcpy(databuf, username);
           databuf += strlen(username);
           }
           else if (Win == 2){
            strcpy(databuf, Remote_username);
           databuf += strlen(Remote_username);
           }

           strcpy(databuf, round);
           databuf += strlen(round);

           if (Win == 1){
           strcpy(databuf, RoundNumber);
           databuf += strlen(RoundNumber);
           }
           else if (Win == 2){
                       strcpy(databuf, round_to_email);
                      databuf += strlen(round_to_email);
           }

           strcpy(databuf, MESSAGEEND);
           databuf += strlen(MESSAGEEND);

           strcpy(databuf, DATAEND);
           databuf += strlen(DATAEND);

    int dataLength = strlen(DATA1);

    strcpy(pcBufHeaders, CTHEADER);
    pcBufHeaders += strlen(CTHEADER);
    strcpy(pcBufHeaders, CLHEADER1);

    pcBufHeaders += strlen(CLHEADER1);
    sprintf(cCLLength, "%d", dataLength);

    strcpy(pcBufHeaders, cCLLength);
    pcBufHeaders += strlen(cCLLength);
    strcpy(pcBufHeaders, CLHEADER2);
    pcBufHeaders += strlen(CLHEADER2);

    strcpy(pcBufHeaders, DATA1);
    pcBufHeaders += strlen(DATA1);

    int testDataLength = strlen(pcBufHeaders);

    UART_PRINT(acSendBuff);


    //
    // Send the packet to the server */
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    return 0;
}

static int http_get(int iTLSSockID){
    char acSendBuff[512];
    char acRecvbuff[1460];
    char cCLLength[200];
    char* pcBufHeaders;
    int lRetVal = 0;

    pcBufHeaders = acSendBuff;
    strcpy(pcBufHeaders, GETHEADER);
    pcBufHeaders += strlen(GETHEADER);
    strcpy(pcBufHeaders, HOSTHEADER);
    pcBufHeaders += strlen(HOSTHEADER);
    strcpy(pcBufHeaders, CHEADER);
    pcBufHeaders += strlen(CHEADER);
    strcpy(pcBufHeaders, "\r\n\r\n");

        char DATA1[512];
       char* databuf = DATA1;

       strcpy(databuf, DATASTART);
           databuf += strlen(DATASTART);

           strcpy(databuf, MESSAGESTART);
           databuf += strlen(MESSAGESTART);

           strcpy(databuf, Local_player);
           databuf += strlen(Local_player);



           strcpy(databuf, username);
           databuf += strlen(username);

           strcpy(databuf, space);
           databuf += strlen(space);

           strcpy(databuf, Remote_player);
           databuf += strlen(Remote_player);

          strcpy(databuf, Remote_username);
           databuf += strlen(Remote_username);

           strcpy(databuf, winnerstatement);
           databuf += strlen(winnerstatement);

           if (Win == 1) {
           strcpy(databuf, username);
           databuf += strlen(username);
           }
           else if (Win == 2){
            strcpy(databuf, Remote_username);
           databuf += strlen(Remote_username);
           }

           strcpy(databuf, round);
           databuf += strlen(round);

           if (Win == 1){
           strcpy(databuf, RoundNumber);
           databuf += strlen(RoundNumber);
           }
           else if (Win == 2){
                                 strcpy(databuf, round_to_email);
                                databuf += strlen(round_to_email);
                     }



           strcpy(databuf, MESSAGEMID);
           databuf += strlen(MESSAGEMID);

           strcpy(databuf, Local_player);
           databuf += strlen(Local_player);



           strcpy(databuf, username);
           databuf += strlen(username);

           strcpy(databuf, space);
           databuf += strlen(space);

           strcpy(databuf, Remote_player);
           databuf += strlen(Remote_player);

           strcpy(databuf, Remote_username);
           databuf += strlen(Remote_username);

           strcpy(databuf, winnerstatement);
           databuf += strlen(winnerstatement);

           if (Win == 1) {
           strcpy(databuf, username);
           databuf += strlen(username);
           }
           else if (Win == 2){
                       strcpy(databuf, Remote_username);
                      databuf += strlen(Remote_username);
                      }


           strcpy(databuf, round);
           databuf += strlen(round);

           if (Win == 1){
           strcpy(databuf, RoundNumber);
           databuf += strlen(RoundNumber);
           }
           else if (Win == 2){
                                 strcpy(databuf, round_to_email);
                                databuf += strlen(round_to_email);
                     }



           strcpy(databuf, MESSAGEEND);
           databuf += strlen(MESSAGEEND);

           strcpy(databuf, DATAEND);
           databuf += strlen(DATAEND);


    int dataLength = strlen(DATA1);

    strcpy(pcBufHeaders, CTHEADER);
    pcBufHeaders += strlen(CTHEADER);
    strcpy(pcBufHeaders, CLHEADER1);

    pcBufHeaders += strlen(CLHEADER1);
    sprintf(cCLLength, "%d", dataLength);

    strcpy(pcBufHeaders, cCLLength);
    pcBufHeaders += strlen(cCLLength);
    strcpy(pcBufHeaders, CLHEADER2);
    pcBufHeaders += strlen(CLHEADER2);

    strcpy(pcBufHeaders, DATA1);
    pcBufHeaders += strlen(DATA1);

    int testDataLength = strlen(pcBufHeaders);

    UART_PRINT(acSendBuff);


    //
    // Send the packet to the server */
    //
    lRetVal = sl_Send(iTLSSockID, acSendBuff, strlen(acSendBuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("POST failed. Error Number: %i\n\r",lRetVal);
        sl_Close(iTLSSockID);
        GPIO_IF_LedOn(MCU_RED_LED_GPIO);
        return lRetVal;
    }
    lRetVal = sl_Recv(iTLSSockID, &acRecvbuff[0], sizeof(acRecvbuff), 0);
    if(lRetVal < 0) {
        UART_PRINT("Received failed. Error Number: %i\n\r",lRetVal);
        //sl_Close(iSSLSockID);
       GPIO_IF_LedOn(MCU_RED_LED_GPIO);
           return lRetVal;
    }
    else {
        acRecvbuff[lRetVal+1] = '\0';
        UART_PRINT(acRecvbuff);
        UART_PRINT("\n\r\n\r");
    }

    return 0;
}
