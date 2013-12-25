#include <stdio.h>
#include <windows.h>

int main()
{
	HINSTANCE hinst;
	hinst = LoadLibrary("CLock.dll");

	typedef short(__stdcall*lpfnConnect)(short);
	lpfnConnect Connect;

	typedef short(__stdcall*lpGetCardNo)(unsigned char*);
	lpGetCardNo getCardNo;

	typedef short(__stdcall*lpDeleteCard)();
	lpDeleteCard deleteCard;

	
	if (hinst != NULL) 
	{
		Connect = (lpfnConnect)GetProcAddress(hinst, "dv_connect");
		Connect(1);


		deleteCard = (lpDeleteCard)GetProcAddress(hinst, "dv_delete_card");
		deleteCard();


		getCardNo = (lpGetCardNo)GetProcAddress(hinst, "dv_get_card_number");

		unsigned char card_no[7] = {0};
		getCardNo(card_no);

		printf("card_no: %s\n", card_no);



		FreeLibrary(hinst);
	}


	return 0;
}