#include "aes.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "stm32f1xx_hal.h"

// Generates and prints 'count' random
// numbers in range [lower, upper].
uint8_t GenerateSeed(int lower, int upper)
{
        int num = (rand() %
        (upper - lower + 1)) + lower;

    return num;
}


void MainEncrupt()
{
	/* The Seed which is gonna be send to the Flashing Tool */
	uint32_t Test = HAL_GetTick();
    srand(Test);
    uint8_t Seed[4] = {0};
    /* Generate Number in between 0 -> 255 */
    Seed[0] = GenerateSeed(0,255);
    Seed[1] = GenerateSeed(0,255);
    Seed[2] = GenerateSeed(0,255);
    Seed[3] = GenerateSeed(0,255);

    /* The Key used to encrypt and decrypt the data */
    /* It is a private key which is only known to the 2 parties " the Tool and the ECU" */
    uint8_t key[] = { 0x2b, 0x7e, 0x15, 0x16};

    struct AES_ctx ctx;

    AES_init_ctx(&ctx, key);
    AES_ECB_encrypt(&ctx, Seed);
//    printf("Seed After Encrypting is : %x %x %x %x\r\n",Seed[0],Seed[1], Seed[2], Seed[3]);

    AES_ECB_decrypt(&ctx, Seed);
//    printf("Seed After Decrypting is : %x %x %x %x\r\n",Seed[0],Seed[1], Seed[2], Seed[3]);


}
