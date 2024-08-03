# Flashing SW in Blue Pill MCU
In this Repo I have implemented:
- PCTool which takes the binary file of the application to send it to Connected ECU through UART following UDS Standard Protocol while being Encrypted using AES128 with different stored Keys which is decided upon which key to use this time during the Seed and Key   
  procedure during request download.
- Used the Linker Script to divide the memory to Bootloader, APP1, and APP2 which will be a backup in case the flashing fails for any reason like being disconnected. I check for the validity of the Flashed Application during booting.
- while receiving the file from the PCTool, the MCU is still working on its tasks so it will not stop working while receiving and flashing the Application Binary Data.
- Having Shared APIs between the Bootloader and Application, so we do not have to implement the same APIs twice which will lead to waste memory.
- Having Shared NON-Init RAM to store flags for example to be used to communicate between the bootloader and Application instead of writing and reading these flags from the flash memory which will be slower.
- Have NV Memory configured which can be used to store some Required Info like the number of flashes, and the root cause of any failure that happens during run time.


It was a wonderful experience implementing this project, I took inspiration from working on professional projects in the automotive market and looking forward to improving it.
Future Modifications:
- Using a Web server to communicate and Flash the SW Over the Air instead of using UART.
- Integrate FreeRTOS and make a bigger application that has a Computer Vision Model on it.
