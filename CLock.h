/**************************************************
File   :  CLock.h
Project:  C¿îËø½Ó¿Ú
Author :  Bollen
Date   :  2013-07-22
***************************************************/
#ifndef __DLL_CLOCK_H
#define __DLL_CLOCK_H

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "dcrf32.h"

#define SUCCESS					 0
#define ERR_INTERFACE			-1
#define FAIL_ENCODER_CONNECT	-2
#define FAIL_ENCODER_REG		-3
#define FAIL_BEEP				-4
#define FAIL_CARD_TYPE			-5
#define ERR_CARD_PWD			-6
#define ERR_CARD_PWD_OBT		-7
#define ERR_CARD_TYPE			-8
#define ERR_AUTH				-9

HANDLE _icdev;

unsigned char mode = 0;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __CLOCK__
#define __CLOCK_LIB_API __declspec(dllexport)
#else
#define __CLOCK_LIB_API __declspec(dllimport)
#endif

//functions declarations
//__CLOCK_LIB_API returntype FuncName (parameters);
//... more declarations as needs

__int16			dv_reg_encoder();
__int16			dv_beep();
void			card_number_gen(unsigned char* card_number);
void			invert(char* str);
__int16			a_to_i(char s[]);
void			int_to_hex(char* dec, char* hex);
void			hex_to_int(char* hex, char* dec);
__int16			dv_check_card();
__int16			dv_read(__int16 type, unsigned char* rd_data /* length:96 */);
__int16			dv_write(__int16 type, unsigned char* data /* length:96 */);
void			parray_to_array(__int16 length, char*parray[], char* array);


__CLOCK_LIB_API	__int16 __stdcall dv_connect(__int16 beep);
__CLOCK_LIB_API	__int16 __stdcall dv_disconnect();
__CLOCK_LIB_API	__int16 __stdcall dv_verify_card(__int16* type);
__CLOCK_LIB_API __int16	__stdcall dv_get_auth_code(unsigned char* auth /*length:6*/);
__CLOCK_LIB_API __int16 __stdcall dv_get_card_number(unsigned char* cardno/*length:6*/);
__CLOCK_LIB_API __int16 __stdcall dv_read_card(unsigned char* auth,
							   unsigned char* cardno,
							   unsigned char* building, 
							   unsigned char* room,
							   unsigned char* commdoors,
							   unsigned char* arrival, 
							   unsigned char* departure);

__CLOCK_LIB_API __int16 __stdcall dv_write_card(unsigned char* auth, 
								unsigned char* building,
								unsigned char* room,
								unsigned char* commdoors,
								unsigned char* arrival,
								unsigned char* departure);

__CLOCK_LIB_API __int16 __stdcall dv_delete_card();


//functions end
#undef __CLOCK_LIB_API

#ifdef __cplusplus
}      
#endif

#endif //__DLL_CLOCK_H
 

