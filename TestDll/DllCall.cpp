#include <stdio.h>
#include <windows.h>

int main()
{
	HINSTANCE hinst;
	typedef short(__stdcall*lpfnConnect)(short);
	typedef short
	lpfnConnect Connect;
	hinst = LoadLibrary("CLock.dll");
	if (hinst != NULL) 
	{
		Connect = (lpfnConnect)GetProcAddress(hinst, "dv_connect");
		Connect(1);
		FreeLibrary(hinst);
	}
	return 0;
}