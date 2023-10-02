#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "RS232\rs232.h"
#include "aes.h"



int comport;
int bdrate   = 115200;       /* 115200 baud */
char mode[]={'8','N','1',0}; /* *-bits, No parity, 1 stop bit */
char bin_name[1024];
int ex = 0;
FILE *Fptr = NULL;

uint8_t DecryptedSeed[16] = {0};

void OpenBinFile(void);

void delay(uint32_t us)
{
    us *= 10;
#ifdef _WIN32
    //Sleep(ms);
    __int64 time1 = 0, time2 = 0, freq = 0;

    QueryPerformanceCounter((LARGE_INTEGER *) &time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    do {
        QueryPerformanceCounter((LARGE_INTEGER *) &time2);
    } while((time2-time1) < us);
#else
    usleep(us);
#endif
}


void OpenBinFile(void)
{
    printf("Opening Binary file : %s\n", bin_name);

    Fptr = fopen(bin_name,"rb");

    if( Fptr == NULL )
    {
      printf("Can not open %s\n", bin_name);
      ex = -1;
      return;
    }

    fseek(Fptr, 0L, SEEK_END);
    uint32_t app_size = ftell(Fptr);
    fseek(Fptr, 0L, SEEK_SET);

    printf("File size = %d\n", app_size);
}

void ReceiveResponseRequestSeed()
{
    /* The Longest Response we can get from the ECU has length = 10 Bytes */
    uint8_t ReceivingBuffer[18] = {0};
    uint8_t Length[1] = {0};
    RS232_PollComport( comport, Length, 1);
    RS232_PollComport( comport, ReceivingBuffer, Length[0]);
    printf("The Response is : ");
    for (uint8_t idx = 0; idx < Length[0]; idx++)
    {
        printf("%x ",ReceivingBuffer[idx]);
    }
    printf("\n");

    struct AES_ctx ctx;
    uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16};
    AES_init_ctx(&ctx, key);
    AES_ECB_decrypt(&ctx, &ReceivingBuffer[2]);

    // printf("After Decryption : ");
    for (uint8_t idx = 2; idx < 18; idx++)
    {
        // printf("%x ", ReceivingBuffer[idx]);
        DecryptedSeed[idx - 2] = ReceivingBuffer[idx];
    }
    // printf("\n");
}

void ReceiveResponseSendKeyRequest()
{
    uint8_t ReceivingBuffer[4] = {0};
    uint8_t Length[1] = {0};
    RS232_PollComport( comport, Length, 1);
    RS232_PollComport( comport, ReceivingBuffer, Length[0]);
    printf("The Response is : ");
    for (uint8_t idx = 0; idx < Length[0]; idx++)
    {
        printf("%x ",ReceivingBuffer[idx]);
    }
    printf("\n");
}

// Function prototypes for menu options
void option1();
void option2();
void option3();
void option4();
void option5();
void option6();
void option7();
void option8();

int main(int argc, char *argv[]) 
{

    comport = atoi(argv[1]) -1;
    int choice;

    if( RS232_OpenComport(comport, bdrate, mode, 0) )
    {
      printf("Can not open comport\n");
      ex = -1;
      return -1;
    }

    do {
        // Display the menu
        printf("****************************** Menu: ******************************\n");
        printf("1. Extended Session\n");
        printf("2. Bootloader Session\n");
        printf("3. Defualt Session \n");
        printf("4. Request Seed \n");
        printf("5. Send Key \n");
        printf("6. Option 6\n");
        printf("7. Option 7\n");
        printf("8. Option 8\n");
        printf("0. Exit\n");

        // Prompt the user for a choice
        printf("Enter your choice: ");
        scanf("%d", &choice);

        // Perform the selected action
        switch (choice) {
            case 1:
                option1();
                break;
            case 2:
                option2();
                break;
            case 3:
                option3();
                break;
            case 4:
                option4();
                break;
            case 5:
                option5();
                break;
            case 6:
                option6();
                break;
            case 7:
                option7();
                break;
            case 8:
                option8();
                break;
            case 0:
                printf("Exiting the program.\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    } while (choice != 0);

    return 0;
}

// Define the functions for each menu option
void option1() 
{
    printf("You selected Option 1.\n");
    /* First Byte is the length of the actual data*/
    uint8_t DataBuffer[2] = {0x10,0x03};
    uint8_t ReceivingBuffer[4];
    /* First Send the Handshake Byte and expect receiving one */
    RS232_SendByte(comport, 0xAA);

    do
    {
        /* code */
        RS232_PollComport( comport, ReceivingBuffer, 1);

    } while (ReceivingBuffer[0] != 0xAA);
    
    // uint8_t Result = RS232_PollComport( comport, ReceivingBuffer, 1);
    
    if(ReceivingBuffer[0] != 0xAA)
    {
        printf("Wrong Value Received\r\n");
        printf("Value Received is %x\n",ReceivingBuffer[0]);
        return;
    }
    RS232_SendByte(comport, 2);
    for (uint8_t idx = 0; idx < 2 ; idx++)
    {   
        delay(1);
        RS232_SendByte(comport, DataBuffer[idx]);
    }
    printf("Finished Sending\n");
    // delay(1000);
    /* Receive the Response from the ECU */
    uint8_t Length[1] = {0};
    RS232_PollComport( comport, Length, 1);
    RS232_PollComport( comport, ReceivingBuffer, Length[0]);
    printf("The Response is : ");
    for (uint8_t idx = 0; idx < Length[0]; idx++)
    {
        printf("%x ",ReceivingBuffer[idx]);
    }
    printf("\n");
}

void option2() 
{
    printf("You selected Option 2.\n");
    /* First Byte is the length of the actual data*/
    uint8_t DataBuffer[2] = {0x10,0x02};
    uint8_t ReceivingBuffer[4];
    /* First Send the Handshake Byte and expect receiving one */
    RS232_SendByte(comport, 0xAA);

    do
    {
        /* code */
        RS232_PollComport( comport, ReceivingBuffer, 1);

    } while (ReceivingBuffer[0] != 0xAA);
    
    // uint8_t Result = RS232_PollComport( comport, ReceivingBuffer, 1);
    
    if(ReceivingBuffer[0] != 0xAA)
    {
        printf("Wrong Value Received\r\n");
        printf("Value Received is %x\n",ReceivingBuffer[0]);
        return;
    }
    RS232_SendByte(comport, 2);
    for (uint8_t idx = 0; idx < 2 ; idx++)
    {   
        delay(1);
        RS232_SendByte(comport, DataBuffer[idx]);
    }
    printf("Finished Sending\n");
    // delay(1000);
    /* Receive the Response from the ECU */
    uint8_t Length[1] = {0};
    RS232_PollComport( comport, Length, 1);
    RS232_PollComport( comport, ReceivingBuffer, Length[0]);
    printf("The Response is : ");
    for (uint8_t idx = 0; idx < Length[0]; idx++)
    {
        printf("%x ",ReceivingBuffer[idx]);
    }
    printf("\n");
}

void option3() 
{
    printf("You selected Option 3.\n");
    /* First Byte is the length of the actual data*/
    uint8_t DataBuffer[2] = {0x10,0x01};
    uint8_t ReceivingBuffer[4];
    /* First Send the Handshake Byte and expect receiving one */
    RS232_SendByte(comport, 0xAA);

    do
    {
        /* code */
        RS232_PollComport( comport, ReceivingBuffer, 1);

    } while (ReceivingBuffer[0] != 0xAA);
    
    // uint8_t Result = RS232_PollComport( comport, ReceivingBuffer, 1);
    
    if(ReceivingBuffer[0] != 0xAA)
    {
        printf("Wrong Value Received\r\n");
        printf("Value Received is %x\n",ReceivingBuffer[0]);
        return;
    }
    RS232_SendByte(comport, 2);
    for (uint8_t idx = 0; idx < 2 ; idx++)
    {   
        delay(1);
        RS232_SendByte(comport, DataBuffer[idx]);
    }
    printf("Finished Sending\n");
    // delay(1000);

    /* Receive the Response from the ECU */
    uint8_t Length[1] = {0};
    RS232_PollComport( comport, Length, 1);
    RS232_PollComport( comport, ReceivingBuffer, Length[0]);
    printf("The Response is : ");
    for (uint8_t idx = 0; idx < Length[0]; idx++)
    {
        printf("%x ",ReceivingBuffer[idx]);
    }
    printf("\n");
}

void option4() 
{
    const uint8_t DataLength = 2;
    uint8_t DataBuffer[2] = {0x27,0x11};
    uint8_t ReceivingBuffer[4];
    /* First Send the Handshake Byte and expect receiving one */
    RS232_SendByte(comport, 0xAA);

    do
    {
        /* code */
        RS232_PollComport( comport, ReceivingBuffer, 1);

    } while (ReceivingBuffer[0] != 0xAA);


    RS232_SendByte(comport, DataLength);
    for (uint8_t idx = 0; idx < DataLength ; idx++)
    {   
        delay(1);
        RS232_SendByte(comport, DataBuffer[idx]);
    }
    ReceiveResponseRequestSeed();

}

void option5() 
{
    const uint8_t DataLength = 2 + 16;
    uint8_t DataBuffer[18] = {0x27,0x12};
    for (uint8_t idx = 0; idx < 16; idx++)
    {
        DataBuffer[idx + 2] = DecryptedSeed[idx];
    }
    uint8_t ReceivingBuffer[4];
    /* First Send the Handshake Byte and expect receiving one */
    RS232_SendByte(comport, 0xAA);

    do
    {
        /* code */
        RS232_PollComport( comport, ReceivingBuffer, 1);

    } while (ReceivingBuffer[0] != 0xAA);

    RS232_SendByte(comport, DataLength);

    for (uint8_t idx = 0; idx < DataLength ; idx++)
    {   
        delay(1);
        RS232_SendByte(comport, DataBuffer[idx]);
    }
    ReceiveResponseSendKeyRequest();

}

void option6() {
    printf("You selected Option 6.\n");
}

void option7() {
    printf("You selected Option 7.\n");
}

void option8() {
    printf("You selected Option 8.\n");
}
