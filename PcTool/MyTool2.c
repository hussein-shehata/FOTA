#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "RS232\rs232.h"
#include "aes.h"



int comport;
int bdrate   = 115200;       /* 115200 baud */
char mode[]={'8','N','1',0}; /* *-bits, No parity, 1 stop bit */
char bin_name[1024] = "D:/0_Projects/FOTA/FOTA/Application/Debug/Application.bin \0";
uint8_t APP_BIN[1024 * 39]; //Max is 39KB
FILE *Fptr = NULL;
uint32_t app_size = 0;

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
      return;
    }


    fseek(Fptr, 0L, SEEK_END);
    app_size = ftell(Fptr);
    fseek(Fptr, 0L, SEEK_SET);

    printf("File size = %d\n", app_size);

            //read the full image
    if( fread( APP_BIN, 1, app_size, Fptr ) != app_size )
    {
      printf("App/FW read Error\n");
      return;
    }
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

void ReceiveResponseMassEraseRequest()
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

void ReceiveResponseDownloadRequest()
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

void ReceiveReponseDataTransfer()
{
    uint8_t ReceivingBuffer[3] = {0};
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
        printf("6. Mass Erase\n");
        printf("7. Data Transfer\n");
        printf("8. Request Download\n");
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

void option6() 
{
    printf("You selected Option 6.\n");
    const uint8_t DataLength = 4;
    uint8_t DataBuffer[4] = {0x31,0x01, 0xFF, 0x00};
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
    ReceiveResponseMassEraseRequest();
}

void option7() 
{
    uint8_t DataTransferComplete = false;
    static uint8_t BlockNumber = 1;
    static uint16_t AccumlativeSentData = 0;
    uint8_t DataToSend = 0;
    uint8_t DataLength = 0;
    uint8_t DataBuffer[128 + 2] = {0x36};
    uint8_t ReceivingBuffer[4];

    while(DataTransferComplete == false)
    {
        DataBuffer[1] = BlockNumber;
        BlockNumber++ ;


        if (app_size - AccumlativeSentData > 128 )
        {
            DataToSend = 128;
        }
        else
        {
            DataToSend = app_size - AccumlativeSentData;
            DataTransferComplete = true; // This the last Frame
        }
        

        DataLength = 2 + DataToSend;


        /* First Send the Handshake Byte and expect receiving one */
        RS232_SendByte(comport, 0xAA);

        do
        {
            /* code */
            RS232_PollComport( comport, ReceivingBuffer, 1);

        } while (ReceivingBuffer[0] != 0xAA);

        RS232_SendByte(comport, DataLength);

        for(uint8_t idx = 0; idx < DataToSend; idx++ )
        {
            DataBuffer[idx + 2] = APP_BIN[AccumlativeSentData + idx];
        }


        for (uint8_t idx = 0; idx < DataLength ; idx++)
        {   
            delay(1);
            RS232_SendByte(comport, DataBuffer[idx]);
        }
        AccumlativeSentData += DataToSend;

        ReceiveReponseDataTransfer();

    }
}

void option8() 
{
    printf("You selected Option 8.\n");
    const uint8_t DataLength = 5;
    // TODO Add the slot option and the encryption Technique
    uint8_t DataBuffer[5] = {0x34, 0x00, 0x00};
    
    uint8_t ReceivingBuffer[2];

    OpenBinFile();
    printf("After Exiting Function \n");
    DataBuffer[3] = (app_size >> 8) & 0xFF; // Take the MSB
    DataBuffer[4] = app_size &0xFF; // Take the LSB
    printf("The LSB %x \n", DataBuffer[4]);
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
        printf(" Data : %x %x %X %X %X", DataBuffer[0], DataBuffer[1], DataBuffer[2], DataBuffer[3], DataBuffer[4]);
    }

    ReceiveResponseDownloadRequest();
}
