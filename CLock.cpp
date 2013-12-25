/**************************************************
File   :  CLock.cpp
Project:  C款锁接口
Author :  Bollen
Date   :  2013-07-22
***************************************************/
#define __CLOCK__

#include "CLock.h"

//--- export functions ------------
__int16 __stdcall dv_connect(__int16 beep)
{
	__int16 result;
	//init device
	_icdev = dc_init(100, 115200);

	if ((__int16)_icdev < 0)
		return FAIL_ENCODER_CONNECT;

	//register encoder
	if ( (result = dv_reg_encoder()) < 0)
		return result;

	//beep: 1
	if (beep == 1)
	{
	   if ((result = dv_beep()) < 0 )
		   return result;
	}

	return SUCCESS;
}

//注：在WIN32环境下icdev为端口的设备句柄，必须释放后才可以再次连接。
__int16 __stdcall dv_disconnect()
{
	return dc_exit(_icdev);
}

__int16 __stdcall dv_check_card()
{
	//card type: 1 (ul), 2 (s50);
	enum _type {unknow=-5, ul=1, m1=2};
	enum _type card_type = unknow;

	unsigned __int16 tagtype;

	if (dc_request(_icdev, mode, &tagtype) !=0 )
		return ERR_INTERFACE;

	switch(tagtype)
	{
	case  4: 
		card_type = m1;
		break;
	case 68: 
		card_type = ul;
		break;
	default:
		card_type = unknow;
		break;
	}

	return card_type;
}

__int16 __stdcall dv_verify_card(__int16* type)
{
	__int16 result;

	unsigned char pwd_mode = 0;

	unsigned char sector = 13;

	char* key = "100000000000";

	*type = dv_check_card();

	if ((result = *type) < 0)
		return result;

	unsigned long snr = 0;

	if (dc_card(_icdev, mode, &snr) != 0)
		return ERR_INTERFACE;

	if (*type == 2) //verify m1 password
	{
		if (dc_load_key_hex(_icdev, pwd_mode, sector, key) != 0)
			return ERR_INTERFACE;

	    if (dc_authentication(_icdev, pwd_mode, sector) != 0)
			return ERR_CARD_PWD;
	}

	unsigned char rd_data[96 + 1] = {0};

	if ((result = dv_read(*type, rd_data)) < 0)
		return result;

	//veriry ORBITA password

	char buf[10] = {0};

	char* ORBITA = "01010205";

	strncpy(buf, (char*)(rd_data + 88), 8);

	if (strcmp(buf, ORBITA) != 0)
		return ERR_CARD_PWD_OBT;

	return SUCCESS;
}

__int16	__stdcall dv_get_auth_code(unsigned char* auth /*lenght:6*/)
{
	__int16 result;

	__int16 type;

	if ((result = dv_verify_card(&type)) < 0)
		return result;
	
	unsigned char rd_data[96 + 1] = {0};

	if ((result = dv_read(type, rd_data)) < 0)
		return result;

	char card_class[3] = {0};

	strncpy(card_class, (char*)(rd_data + 6), 2);

	if (strcmp(card_class,"0A") != 0)
		return ERR_CARD_TYPE;

	strncpy((char*)auth, (char*)rd_data, 6);

	return SUCCESS;
}

__int16 __stdcall dv_get_card_number(unsigned char* cardno/*length:6*/)
{
	__int16 result;

	__int16 type;

	if((result = dv_verify_card(&type)) < 0)
		return result;

	unsigned char rd_data[96 + 1] = {0};

	if ((result = dv_read(type, rd_data)) < 0)
		return result;

    //--

	char card_type[4] = {0};
	
	strncpy(card_type, (char*)(rd_data + 6), 2);

	if (strcmp(card_type,"0A") == 0)     //setup card
	{
		strncpy((char*)cardno, (char*)(rd_data + 32), 6);    
	}
	else if (strcmp(card_type, "0B") == 0)  //clock card
	{
		strncpy((char*)cardno, (char*)(rd_data + 24), 6);  
	}
	else if (strcmp(card_type, "0C") == 0)  //master card
	{
		strncpy((char*)cardno, (char*)(rd_data + 8), 6);
	}
	else if (strcmp(card_type, "0D") == 0 || strcmp(card_type, "0E") == 0) //guest card
	{
		strncpy((char*)cardno, (char*)(rd_data + 18), 6);
	}
	else
	{
		cardno = '\0';
	}

	//--


	return SUCCESS;	
}

__int16 __stdcall dv_read_card(unsigned char* auth,
							   unsigned char* cardno,
							   unsigned char* building, 
							   unsigned char* room,
							   unsigned char* commdoors,
							   unsigned char* arrival, 
							   unsigned char* departure)
{
	__int16 result;

	//verify card
    __int16 type;

	if ((result = dv_verify_card(&type)) < 0)
		return result;

	unsigned char rd_data[96 + 1] = {0};

	if ((result = dv_read(type, rd_data)) < 0)
		return result;

	char szauth[10] = {0};

	strncpy(szauth, (char*)rd_data, 6);

	if (strcmp(szauth, (char*)auth) != 0)
		return ERR_AUTH;

	char sztype[10] = {0};

	strncpy(sztype, (char*)(rd_data + 6), 2);

	if (strcmp(sztype,"0D") != 0)
		return ERR_CARD_TYPE;

	strncpy((char*)cardno, (char*)(rd_data + 18), 6);

	//building
	char szbuilding[10] = {0};

	strncpy(szbuilding, (char*)(rd_data + 10), 2);

	hex_to_int(szbuilding, (char*)building);

	//room
	char hexfloor[10] = {0};
	char szfloor[10] = {0};

	strncpy(hexfloor, (char*)(rd_data + 12), 2);

	hex_to_int(hexfloor, szfloor);

	char hexroom[10] = {0};
	char szroom[10] = {0};

	strncpy(hexroom, (char*)(rd_data + 64), 2);

	hex_to_int(hexroom, szroom);

	sprintf((char*)room, "%.2s%.2s", szfloor, szroom);

	//common doors
	char szcommdoors[10] = {0};

	strncpy(szcommdoors, (char*)(rd_data + 30), 2);

	hex_to_int(szcommdoors, (char*)commdoors);

	//start date
	char year1[5] = {0};
	char month1[5] = {0};
	char day1[5] = {0};
	char hour1[5] = {0};
	char minute1[5] = {0};
	char second1[5] = {0};

	strncpy(year1, (char*)(rd_data + 42), 2);
	strncpy(month1, (char*)(rd_data + 40), 2);
	strncpy(day1, (char*)(rd_data + 38), 2);
	strncpy(hour1, (char*)(rd_data + 36), 2);
	strncpy(minute1, (char*)(rd_data + 34), 2);
	strncpy(second1, (char*)(rd_data + 32), 2);

	char tmp_arrival[20] = {0};

	sprintf(tmp_arrival, "20%.2s-%.2s-%.2s %.2s:%.2s:%.2s", year1, 
		month1, day1, hour1, minute1, second1);

	struct tm tm1;
	time_t time1;
	sscanf(tmp_arrival, "%4d-%2d-%2d %2d:%2d:%2d",
		&tm1.tm_year,
		&tm1.tm_mon,
		&tm1.tm_mday,
		&tm1.tm_hour,
		&tm1.tm_min,
		&tm1.tm_sec);
	
	tm1.tm_year -= 1900;

	tm1.tm_mon --;
	
	tm1.tm_isdst = -1;

	time1 = mktime(&tm1);

	time1 += (100 * 24 * 3600);

	struct tm tm2;

#ifdef WIN32
	tm2 = *localtime(&time1);
#else
	localtime_r(&time1, &tm2);
#endif

	sprintf((char*)arrival, "%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d",
		tm2.tm_year + 1900,
		tm2.tm_mon + 1,
		tm2.tm_mday,
		tm2.tm_hour,
		tm2.tm_min,
		tm2.tm_sec);

	//end date
	char year2[5] = {0};
	char month2[5] = {0};
	char day2[5] = {0};
	char hour2[5] = {0};
	char minute2[5] = {0};
	char second2[5] = {0};
	
	strncpy(year2, (char*)(rd_data + 54), 2);
	strncpy(month2, (char*)(rd_data + 52), 2);
	strncpy(day2, (char*)(rd_data + 50), 2);
	strncpy(hour2, (char*)(rd_data + 48), 2);
	strncpy(minute2, (char*)(rd_data + 46), 2);
	strncpy(second2, (char*)(rd_data + 44), 2);

	sprintf((char*)departure, "20%.2s-%.2s-%.2s %.2s:%.2s:%.2s", year2, 
		month2, day2, hour2, minute2, second2);

	dv_beep();

	return SUCCESS;
}

__int16 __stdcall dv_write_card(unsigned char* auth, 
								unsigned char* building,
								unsigned char* room,
								unsigned char* commdoors,
								unsigned char* arrival,
								unsigned char* departure)
{

	__int16 result;

	//verify card
	__int16 type;

	if ((result = dv_verify_card(&type)) < 0)
		return result;

	char *data[] = {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","01","01","02","05"};

	//auth code
	char auth1[3] = {0};
	char auth2[3] = {0};
	char auth3[3] = {0};

	strncpy(auth1, (char*)auth, 2);
	strncpy(auth2, (char*)auth + 2 , 2);
	strncpy(auth3, (char*)auth + 4, 2);

	data[0] = auth1;
	data[1] = auth2;
	data[2] = auth3;

	//card type
	data[3] = "0D";

	//building
	char szbuilding[4] = {0};

	int_to_hex((char*)building, szbuilding);

	data[5] = szbuilding;
    
	//floor room
	char room_number[5] = {0};

	sprintf(room_number, "%04.4s", room);

	char szfloor[4] = {0};

	char szroom[4] = {0};

	char szfloor2[4] = {0};

	char szroom2[4] = {0};

	strncpy(szfloor, room_number, 2);

	strncpy(szroom, room_number + 2, 2);

	int_to_hex(szfloor, szfloor2);

	data[6] = szfloor2;

	int_to_hex(szroom, szroom2);

	data[32] = szroom2;

	//common doors
	char szcommdoors[4] = {0};

	int_to_hex((char*)commdoors, szcommdoors);

	data[15] = szcommdoors;

	//card number
	unsigned char cardno[7] = {0};

	card_number_gen(cardno);

	char cardno1[4] = {0};

	char cardno2[4] = {0};

	char cardno3[4] = {0};

	strncpy(cardno1, (char*)cardno, 2);

	data[9] = cardno1;

	strncpy(cardno2, (char*)(cardno + 2), 2);

	data[10] = cardno2;

	strncpy(cardno3, (char*)(cardno + 4), 2);

	data[11] = cardno3;
    
	//start date
	struct tm tm1;

	time_t time1;

	sscanf((char*)arrival, "%d-%d-%d %d:%d:%d", 
		&tm1.tm_year, 
		&tm1.tm_mon, 
		&tm1.tm_mday, 
		&tm1.tm_hour, 
		&tm1.tm_min, 
		&tm1.tm_sec);

	tm1.tm_year -= 1900;

	tm1.tm_mon --;

	tm1.tm_isdst = -1;

	time1 = mktime(&tm1);

	time1 -= (100 * 24 * 3600);

	struct tm tm2;

#ifdef WIN32
	tm2 = *localtime(&time1);
#else
	localtime_r(&time1, &tm2);
#endif

	char szyear[5] = {0};
	char szyear_2[3] = {0};
	char szmonth[3] = {0};
	char szday[3] = {0};
	char szhour[3] = {0};
	char szminute[3] = {0};
	char szsec[3] = {0};

	sprintf(szyear, "%4.4d", tm2.tm_year + 1900);
	sprintf(szmonth, "%2.2d", tm2.tm_mon + 1);
	sprintf(szday, "%2.2d", tm2.tm_mday);
	sprintf(szhour, "%2.2d", tm2.tm_hour);
	sprintf(szminute, "%2.2d", tm2.tm_min);
	sprintf(szsec, "%2.2d", tm2.tm_sec);

	strncpy(szyear_2, szyear + 2, 2);

	data[16] = szsec;
	data[17] = szminute;
	data[18] = szhour;
	data[19] = szday;
	data[20] = szmonth;
	data[21] = szyear_2;


	//end date
	__int16 year, month, day, hour, minute, sec;

	sscanf((char*)departure, "%d-%d-%d %d:%d:%d", 
		&year, &month, &day, &hour, &minute, &sec);

	char szyear2[5] = {0};
	char szyear2_2[3] = {0};
	char szmonth2[3] = {0};
	char szday2[3] = {0};
	char szhour2[3] = {0};
	char szminute2[3] = {0};
	char szsec2[3] = {0};

	sprintf(szyear2, "%4.4d", year);
	sprintf(szmonth2, "%2.2d", month);
	sprintf(szday2, "%2.2d", day);
	sprintf(szhour2, "%2.2d", hour);
	sprintf(szminute2, "%2.2d", minute);
	sprintf(szsec2, "%2.2d", sec);

	strncpy(szyear2_2, szyear2 + 2, 2);

	data[22] = szsec2;
	data[23] = szminute2;
	data[24] = szhour2;
	data[25] = szday2;
	data[26] = szmonth2;
	data[27] = szyear2_2;

	unsigned char rd_data[97] = {0};

	parray_to_array(48, data, (char*)rd_data);

	//write data
	if ((result = dv_write(type, rd_data)) < 0)
		return result;

	dv_beep();

	return SUCCESS;
}

__int16 __stdcall dv_delete_card()
{
	__int16 result;

	//verify card
	__int16 type;

	if ((result = dv_verify_card(&type)) < 0)
		return result;

	char *data[] = {"00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","00","01","01","02","05"};

	unsigned char rd_data[97] = {0};

	parray_to_array(48, data, (char*)rd_data);

	//write data
	if ((result = dv_write(type, rd_data)) < 0)
		return result;

	dv_beep();

	return SUCCESS;
}

//--- auxiliary function ------------

__int16 dv_reg_encoder()
{
	const char* regno = "OBTWXY070501";

	unsigned char buffer[32];

	if (dc_srd_eeprom(_icdev, 0, 32, buffer) != 0)
		return ERR_INTERFACE;

	if (strcmp((const char*)buffer, regno) != 0)
		return FAIL_ENCODER_REG;

	return SUCCESS;
}

__int16 dv_beep()
{
	if (dc_beep(_icdev,10) != 0)
		return FAIL_BEEP;

	return SUCCESS;
}

void card_number_gen(unsigned char* card_number)
{
	__int16 i = 0;

	__int16 X = 1, Y = 9;

	time_t t;

	char buf[2] = {0};
	
	srand((unsigned)time(&t));

	for (i = 0; i < 6; i++)
	{
		__int16 val = rand() % (Y - X + 1) + X;

		sprintf(buf, "%d", val);

		memcpy(card_number + i * strlen(buf), buf, strlen(buf));
	}

}


void invert(char* str)
{
	__int16 len = strlen(str);

	__int16 times = len / 2;

    char tmp;

	__int16 i;

	for (i = 0; i < times; i++)
	{
		tmp = str[i];

		str[i] = str[len - i - 1];

		str[len - i - 1] = tmp;
	}
}

__int16 a_to_i(char s[])
{
	__int16 n = 0;

	__int16 i;

	for (i = 0; s[i] >= '0' && s[i] <= '9'; i++)
	{
		n = 10 * n + (s[i] - '0');
	}

	return n;
}

void int_to_hex(char* dec, char* hex)
{	
	__int16 _int = a_to_i(dec);

	if (_int <= 0) return;

	char tmp;

	char buf[2] = {0};

    __int16 i = 0, rem;

	while (_int > 0)
	{	  
      rem = _int % 16;

      switch (rem)
	  {
	  case 10:
		  tmp = 'A';
		  break;
	  case 11:
		  tmp = 'B';
		  break;
	  case 12:
		  tmp = 'C';
		  break;
	  case 13:
		  tmp = 'D';
		  break;
	  case 14:
		  tmp = 'E';
		  break;
	  case 15:
		  tmp = 'F';
		  break;
	  default:
		  sprintf(buf, "%d", rem);
		  tmp = buf[0];
		  break;
	  }
      
	  memcpy(hex + i * sizeof(char), &tmp, sizeof(tmp));

	  i = i + 1;

      _int = _int / 16;
	 
	}

	if (strlen(hex) == 1) strcat(hex,"0");

	invert(hex);
}

void hex_to_int(char* hex, char* dec)
{
	__int16 len = strlen(hex);

	__int16 _int = 0;


	for (; len > 0; len--, hex++)
	{
		char digit = hex[0];

		if (digit >= '0' && digit <= '9') digit = (digit - '0');
		else if (digit >= 'A' && digit <= 'F') digit = (digit - 'A') + 10;
		else if (digit >= 'a' && digit <= 'f') digit = (digit - 'a') + 10;
		else return;

		_int = ((_int << 4) | digit);
	}

	sprintf(dec, "%2.2d", _int);
}

__int16 dv_read(__int16 type, unsigned char* rd_data /* length:96 */)
{
	
	char data[32 + 1] = {0};

	if (type == 1) //ul
	{
		__int16 i = 4;

		__int16 p = 0;

		while (i < 16)
		{
			if (dc_read_hex(_icdev, (unsigned char)i, data) != 0)
				return ERR_INTERFACE;

			memcpy(rd_data + p * 32, data, 32);

			p++;

			i = i + 4;
		}
	}
	else if(type == 2) //m1
	{
		__int16 i = 52;
		__int16 p = 0;

		while (i <= 54)
		{
			if (dc_read_hex(_icdev, (unsigned char)i, data) != 0)
				return ERR_INTERFACE;

			memcpy(rd_data + p * 32, data, 32);
			
			p++;

			i++;
		}
	}

	return SUCCESS;
}


__int16 dv_write(__int16 type, unsigned char* data /* length:96 */)
{
    unsigned char addr;

	unsigned char buffer[33] = {0};

	if (type == 1)
	{
		__int16 i;

		unsigned char fill[] = "000000000000000000000000";

		__int16 p = 0;

		for (i = 1; i < 13; i++)
		{
			memset(buffer, 0, 33);

			strncpy((char*)buffer, (char*)(data + p * 8), 8);

			strcat((char*)buffer, (char*)fill);

            addr = (unsigned char)(i + 3);

            if (dc_write_hex(_icdev, addr, (char*)buffer) != 0)
				return ERR_INTERFACE;

			p++;
		}
	}
	else if(type == 2)
	{
		__int16 i;

		__int16 p = 0;

		for (i = 1; i < 4; i++)
        {
			memset(buffer, 0, 33);		

			strncpy((char*)buffer, (char*)(data + p * 32), 32);	

            addr = (unsigned char)(i + 51);

            if (dc_write_hex(_icdev, addr, (char*)buffer) != 0)
				return ERR_INTERFACE;

			p++;
       }
	}

	return SUCCESS;
}

void parray_to_array(__int16 length, char*parray[], char* array)
{
	__int16 i;

	for (i = 0; i < length; i++)
	{
		strncpy(array + i * 2, *(parray + i), 2);
	}
}