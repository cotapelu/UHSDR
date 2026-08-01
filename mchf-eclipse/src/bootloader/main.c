/**
 * @brief Bootloader main entry point
 * 
 * This file provides the main() function required by the startup code.
 * It calls the actual bootloader entry point Bootloader_Main().
 */

#include "bootloader_main.h"

int main(void)
{
    return Bootloader_Main();
}