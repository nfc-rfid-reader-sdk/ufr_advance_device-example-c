/*
 ============================================================================
 Project Name: uFR Advance C console example
 Name        : main.c
 Author      : d-logic (http://www.d-logic.net/nfc-rfid-reader-sdk/)
 Version     :
 Copyright   : 2019
 Description : Project in C (Language standard: c99)
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#if __WIN32 || __WIN64
#	include <conio.h>
#	include <windows.h>
#elif linux || __linux__ || __APPLE__
#	define __USE_MISC
#	include <unistd.h>
#	include <termios.h>
#	undef __USE_MISC
#	include "conio_gnu.h"
#else
#	error "Unknown build platform."
#endif
#include "../lib/include/uFCoder.h"
#include "ini.h"
#include "uFR.h"
#include "utils.h"
//------------------------------------------------------------------------------
void usage(void);
void menu(char key);
void getCardInfo(void);
void getReaderTime(void);
void setReaderTime(void);
void readerEEPROMRead(void);
void readerEEPROMWrite(void);
void readerChangePassword(void);
//------------------------------------------------------------------------------
int main(void)
{
	char key;
	UFR_STATUS status;
    int choice = 0;

	printf("Select reader opening mode\n");
	printf("  (1) - Simple Reader open\n");
	printf("  (2) - Advanced Reader open\n");
    scanf("%d", &choice);
    if((choice > 2)||(choice <= 0)){
        printf("Invalid selection.\n");
        printf("Press ENTER to quit\n");
        getchar();
        return 0;
    }

    if (choice == 1){
        status = ReaderOpen();

    } else if (choice == 2){
          uint32_t reader_type = 1;
          char port_name[1024] = "";
	      uint32_t port_interface = 2;
	      char open_args[1024] = "";

          printf("Enter reader type:\n");
          scanf("%d", &reader_type);
          fflush(stdin);
          printf("Enter port name:\n");
          scanf("%s", port_name);
          fflush(stdin);
          printf("Enter port interface:\n");
          scanf("%d", port_interface);
          fflush(stdin);
          printf("Enter additional argument:\n");
          scanf("%s",open_args);
          fflush(stdin);

          status = ReaderOpenEx(reader_type,port_name,port_interface,open_args);

          /** TODO: TEST THIS FUNCTION WITH UFR ONLINE **/
    }

	if (status != UFR_OK)
	{
		printf("Error while opening device, status is: %s\n", UFR_Status2String(status));
		printf("Press ENTER to quit\n");
		fflush(stdin);
		getchar();

		return EXIT_FAILURE;
	}

#if __WIN32 || __WIN64
	Sleep(500);
#else // if linux || __linux__ || __APPLE__
	usleep(500000);
#endif
    usage();

	if (!CheckDependencies())
	{
		ReaderClose();
		getchar();
		return EXIT_FAILURE;
	}

	printf(" --------------------------------------------------\n");
	printf("        uFR NFC reader successfully opened.\n");
	printf(" --------------------------------------------------\n");

	do
	{
        #if linux || __linux__ || __APPLE__
            key = getchar();
        #endif
		#if __WIN32 || __WIN64
            key = _getch();
        #endif
		menu(key);
	}
	while (key != '\x1b');

	ReaderClose();
#if linux || __linux__ || __APPLE__
	//_resetTermios();
	//tcflush(0, TCIFLUSH); // Clear stdin to prevent characters appearing on prompt
#endif
	return EXIT_SUCCESS;
}
//------------------------------------------------------------------------------
void menu(char key)
{
	switch (key)
	{
		case '1':
			getCardInfo();
			break;

		case '2':
			getReaderTime();
			break;

		case '3':
			setReaderTime();
			break;

        case '4':
			readerEEPROMRead();
			break;

        case '5':
			readerEEPROMWrite();
			break;

        case '6':
			readerChangePassword();
			break;

		case '\x1b':
			break;

		default:
			usage();
			break;
	}

	printf(" --------------------------------------------------\n");
}
//------------------------------------------------------------------------------
void usage(void)
{
		printf(" +------------------------------------------------+\n"
			   " |           uFR Advance console example          |\n"
			   " |              version "APP_VERSION"                       |\n"
			   " +------------------------------------------------+\n"
			   #if linux || __linux__ || __APPLE__

			   "                              For exit, hit ESC then ENTER.\n");
               #endif
               #if __WIN32 || __WIN64
			   "                              For exit, hit ESC.\n");
               #endif
		printf(" --------------------------------------------------\n");
		printf("  (1) - Get card info\n"
			   "  (2) - Get reader time\n"
			   "  (3) - Set reader time\n"
			   "  (4) - Reader EEPROM read\n"
			   "  (5) - Reader EEPROM write\n"
			   "  (6) - Change reader password\n");
}
//------------------------------------------------------------------------------
void getCardInfo(void)
{
	UFR_STATUS status;
	uint8_t sak = 0;
    uint8_t uid[10] = {0};
    uint8_t uid_size = 0;
	status = GetCardIdEx(&sak, uid, &uid_size);
	if (status == UFR_OK){
        printf("CARD UID -> ");
        print_hex_ln(uid,uid_size,":");
    } else{
          printf("Getting card info failed.\n");
          printf("Status: %s\n", UFR_Status2String(status));
	}
}
//------------------------------------------------------------------------------
void getReaderTime(void)
{
    UFR_STATUS status;
    uint8_t time[6] = {0};
    status = GetReaderTime(time);
    if (status == 0){
        printf("Currently set reader time is:\n");
        printf("Year   - 20%02d \n", time[0]);
        printf("Month  - %02d\n", time[1]);
        printf("Day    - %02d\n", time[2]);
        printf("Hour   - %02d\n", time[3]);
        printf("Minute - %02d\n", time[4]);
        printf("Second - %02d\n", time[5]);
    }else{
        printf("Could not get reader time.\n");
        printf("Status: %s\n", UFR_Status2String(status));
    }
}
//------------------------------------------------------------------------------
void setReaderTime(void)
{
    UFR_STATUS status = 0;
    uint8_t to_set[6] = {0};
    int choice = 0;
    printf("Set reader time options:\n");
    printf("   (1) - Set current PC time\n");
    printf("   (2) - Set reader time manually\n");
    scanf("%d", &choice);

    if((choice > 2)||(choice <= 0)){
        printf("Invalid selection. Returning to main..\n");
        return;
    }

    if (choice == 1){
        uint8_t input[1024];
        uint8_t pwd[8] = {0};

        printf("Enter reader password(8 characters, e.g 11111111):\n");
        scanf("%s", input);
        if (strlen(input) > 8){
            printf("Password must be 8 characters long. Try again:\n");
            scanf("%s", input);
            if (strlen(input) > 8){
                printf("Invalid input. Returning to main...\n");
                return;
            }
        }
        for (int x = 0; x < 8; x++){
            pwd[x] = input[x]; //preparing shorter, necessary, array for our function
        }
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        to_set[0] = (uint8_t)(tm.tm_year-100);
        to_set[1] = (uint8_t)(tm.tm_mon);
        to_set[2] = (uint8_t)(tm.tm_mday);
        to_set[3] = (uint8_t)(tm.tm_hour);
        to_set[4] = (uint8_t)(tm.tm_min);
        to_set[5] = (uint8_t)(tm.tm_sec);

        status = SetReaderTime(pwd, to_set);

        if (status == 0){
            printf("Successfully set reader time.\n");
            printf("Status: %s\n", UFR_Status2String(status));
        } else{
            printf("Could not set reader time.\n");
            printf("Status: %s\n", UFR_Status2String(status));
        }
    } else if (choice == 2){
        uint8_t input[1024];
        uint8_t pwd[8] = {0};
        uint8_t to_set[6] = {0};
        int set_year = 0, set_month = 0, set_day = 0;
        int set_hour = 0, set_min = 0, set_sec = 0;
        printf("Enter reader password(8 characters, e.g 11111111):\n");
        scanf("%s", input);
        if (strlen(input) > 8){
            printf("Password must be 8 characters long. Try again:\n");
            scanf("%s", input);
            if (strlen(input) > 8){
                printf("Invalid input. Returning to main...\n");
                return;
            }
        }
        for (int x = 0; x < 8; x++){
            pwd[x] = input[x]; //preparing shorter, necessary, array for our function
        }

        printf("Enter Year you wish to set(1 byte, e.g 19 - 2019. 18 - 2018 etc...):\n");
        scanf(" %d", &set_year);
        to_set[0] = (uint8_t)set_year;

        printf("Enter Month you wish to set(1 byte, e.g 1 - January, 5 - May etc..):\n");
        scanf(" %d", &set_month);
        to_set[1] = (uint8_t)set_month;

        printf("Enter Day you wish to set (1 byte, 1-31):\n");
        scanf(" %d", &set_day);
        to_set[2] = (uint8_t)set_day;

        printf("Enter Hour you wish to set (1 byte, 0-23):\n");
        scanf(" %d", &set_hour);
        to_set[3] = (uint8_t)set_hour;

        printf("Enter Minute you wish to set (1 byte, 0-59):\n");
        scanf(" %d", &set_min);
        to_set[4] = (uint8_t)set_min;

        printf("Enter Second you wish to set (1 byte, 0-59):\n");
        scanf(" %d", &set_sec);
        to_set[5] = (uint8_t)set_sec;

        status = SetReaderTime(pwd, to_set);

        if (status == 0){
            printf("Successfully set reader time.\n");
            printf("Status: %s\n", UFR_Status2String(status));
        } else{
            printf("Could not set reader time.\n");
            printf("Status: %s\n", UFR_Status2String(status));
        }
    }
}
//------------------------------------------------------------------------------
void readerEEPROMRead(void)
{
    UFR_STATUS status;
    uint32_t address = 0, size = 0;

    printf("Enter EEPROM address from which you wish to start reading:\n");
    scanf("%d", &address);

    printf("Enter how many bytes to read:\n");
    scanf("%d", &size);

    uint8_t data[size];
    memset(data, 0, size);

    status = ReaderEepromRead(data, address, size);

    if (status == 0){
        printf("EEPROM data successfully read.\n");
        printf("DATA:\n");
        for (int x = 0; x < size; x++){
            if (x!=0){
                if (x%32 == 0){
                    printf("\n");
                }
            }
            printf("%02X ", data[x]);
        }
        printf("\n");
    } else {
          printf("Could not read EEPROM data.\n");
          printf("Status: %s\n", UFR_Status2String(status));
    }
}
//------------------------------------------------------------------------------
void readerEEPROMWrite(void)
{
    UFR_STATUS status;
    uint8_t data_input[1024] = {0};
    uint8_t data[128] = {0};
    uint8_t pwd_input[1024] = {0};
    uint8_t pwd[8] = {0};
    uint32_t address = 0, size = 0;

    printf("Enter EEPROM address from which you wish to start writing:\n");
    scanf("%d", &address);

    printf("Enter data you wish to write into EEPROM:\n");
    scanf("%s", data_input);
    if (strlen(data_input) > 256){
        printf("Maximum length of data written can be 128 bytes. Try again:\n");
        scanf("%s", data_input);
        if (strlen(data_input) > 256){
            printf("Invalid input. Returning to main...\n");
            return;
        }
    }
    hex2bin(data, data_input);
    size = strlen(data);

    printf("Enter reader password(8 characters, e.g 11111111):\n");
    scanf("%s", pwd_input);
    if (strlen(pwd_input) > 8){
        printf("Password must be 8 characters long. Try again:\n");
        scanf("%s", pwd_input);
        if (strlen(pwd_input) > 8){
            printf("Invalid input. Returning to main...\n");
            return;
        }
    }
    for (int x = 0; x < 8; x++){
        pwd[x] = pwd_input[x]; //preparing shorter, necessary, array for our function
    }

    status = ReaderEepromWrite(data, address, size, pwd);
    if (status == 0){
        printf("EEPROM data successfully written.\n");
        printf("Status: %s\n", UFR_Status2String(status));
    } else{
          printf("Could not write EEPROM data.\n");
          printf("Status: %s\n", UFR_Status2String(status));
    }
}
//------------------------------------------------------------------------------
void readerChangePassword(void)
{
    UFR_STATUS status;
    uint8_t old_pwd_input[1024] = {0};
    uint8_t old_pwd[8] = {0};

    uint8_t new_pwd_input[1024] = {0};
    uint8_t new_pwd[8] = {0};


    printf("Enter old reader password(8 characters):\n");
    scanf("%s", old_pwd_input);
    if (strlen(old_pwd_input) > 8){
        printf("Password must be 8 characters long. Try again:\n");
        scanf("%s", old_pwd_input);
        if (strlen(old_pwd_input) > 8){
            printf("Invalid input. Returning to main...\n");
            return;
        }
    }
    for (int x = 0; x < 8; x++){
        old_pwd[x] = old_pwd_input[x]; //preparing shorter, necessary, array for our function
    }

    printf("Enter new reader password(8 characters):\n");
    scanf("%s", new_pwd_input);
    if (strlen(new_pwd_input) > 8){
        printf("Password must be 8 characters long. Try again:\n");
        scanf("%s", new_pwd_input);
        if (strlen(new_pwd_input) > 8){
            printf("Invalid input. Returning to main...\n");
            return;
        }
    }
    for (int x = 0; x < 8; x++){
        new_pwd[x] = new_pwd_input[x]; //preparing shorter, necessary, array for our function
    }

    status = ChangeReaderPassword(old_pwd, new_pwd);

    if (status == 0){
        printf("Reader password successfully changed.\n");
        printf("Status: %s\n", UFR_Status2String(status));
    } else {
        printf("Could not change reader password.\n");
        printf("Status: %s\n", UFR_Status2String(status));
    }
}
