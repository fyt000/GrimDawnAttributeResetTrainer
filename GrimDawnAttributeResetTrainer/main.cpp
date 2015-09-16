#include <iostream>
#include <windows.h>
#include <string>
#include <ctime>
#include <tchar.h>
#include <TlHelp32.h> 
#include <Psapi.h>
#include <wchar.h>
#include <cstdint>
#include <string>
#pragma comment( lib, "psapi" )
#pragma hdrstop

using namespace std;


int memError=0;
int success=0;
//windows proc
LRESULT CALLBACK WindowProcedure(HWND,UINT,WPARAM,LPARAM);

#define IDBUTTON 102


DWORD physiqueOffsets[]={0x68,0x964,0x0,0x8,0x0};
DWORD cunningOffsets[]={0x68,0x964,0x8,0x8,0x0};
DWORD spiritOffsets[]={0x68,0x964,0x4,0x8,0x0};
DWORD attributeOffsets[]={0x68,0xdac};

char szClassName[]="GD Attribute Resetter for B27H2";
HINSTANCE g_hInst;


DWORD getFinalAddress(DWORD procID,HANDLE procHandle,DWORD baseAddress, DWORD offsets[], int offsetSize){

	BYTE* exeBaseAddress=0;

    void* hSnap=0;
    MODULEENTRY32 Mod32={0};

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,procID);
   
    Mod32.dwSize = sizeof(MODULEENTRY32);
    while (Module32Next(hSnap, &Mod32)){
        if (!_stricmp("Grim Dawn.exe", Mod32.szModule)){
            CloseHandle(hSnap);
            exeBaseAddress=Mod32.modBaseAddr;
			break;
        }
    }

    CloseHandle(hSnap);

	DWORD ptr=0;
	DWORD memPtr=0;
	//cout<<hex<<(void*)exeBaseAddress<<endl;
	if (!ReadProcessMemory(procHandle,(void*)(exeBaseAddress+baseAddress),&memPtr,4,NULL)){
		memError=1;
		//cout<<"failed to read memory "<<dec<<GetLastError()<<endl;
		//exit(0);
		return 0;
	}

	for (int i=0;i<offsetSize;i++){
		ptr=memPtr+offsets[i];
		//cout<<"Accessing: "<<hex<<ptr<<endl;
		if (!ReadProcessMemory(procHandle,(void*)ptr,&memPtr,4,NULL)){
			memError=1;
			return 0;
		}
		//cout<<"Address: "<<hex<<memPtr<<endl;
	}
	return ptr;
}


string tryResetAttribute(){
	success=0;
	memError=0;
	HWND hGameWindow = NULL;
	DWORD procID= NULL;
	HANDLE procHandle=NULL;

	hGameWindow=FindWindow(NULL,"Grim Dawn");

	if (hGameWindow){
		GetWindowThreadProcessId(hGameWindow,&procID);
		if (procID){
			procHandle=OpenProcess(PROCESS_ALL_ACCESS,false,procID);
			if (procHandle==INVALID_HANDLE_VALUE||!procHandle){
				CloseHandle(procHandle);
				return "Failed to open process handle.";
			}
		}
		else{
			CloseHandle(procHandle);
			return "Failed to find process ID.";
		}

	}
	else{
		CloseHandle(procHandle);
		return "Grim Dawn not running.";
	}


	float pVal;
	float cVal;
	float sVal;
	int aVal;
	DWORD pAddr=getFinalAddress(procID,procHandle,0x230230,physiqueOffsets,5);
	DWORD cAddr=getFinalAddress(procID,procHandle,0x230230,cunningOffsets,5);
	DWORD sAddr=getFinalAddress(procID,procHandle,0x230230,spiritOffsets,5);
	DWORD aAddr=getFinalAddress(procID,procHandle,0x230230,attributeOffsets,2);
	if (memError){
		CloseHandle(procHandle);
		return "Failed to retrieve memory.\nMake sure you have logged into your character.\nOtherwise, please contact the author.";
	}
	int readResult;
	readResult=ReadProcessMemory(procHandle,(void*)pAddr,&pVal,sizeof(pVal),0);
	readResult=ReadProcessMemory(procHandle,(void*)cAddr,&cVal,sizeof(cVal),0)&&readResult;
	readResult=ReadProcessMemory(procHandle,(void*)sAddr,&sVal,sizeof(sVal),0)&&readResult;
	readResult=ReadProcessMemory(procHandle,(void*)aAddr,&aVal,sizeof(aVal),0)&&readResult;
	if (!readResult){
		CloseHandle(procHandle);
		return "Failed to retrieve memory.\nMake sure you have logged into your character.\nOtherwise, please contact the author.";
	}
	/*
	cout<<"Current available points: "<<aVal<<endl;
	cout<<"Current physique: "<<pVal<<endl;
	cout<<"Current cunning: "<<cVal<<endl;
	cout<<"Current spirit: "<<sVal<<endl;
	*/
	if (int(pVal-50)%8||int(cVal-50)%8||int(sVal-50)%8){
		CloseHandle(procHandle);
		return "One of your attribute is abnormal. You probably cheated.";
	}

	float fifty=50;
	int newAttr=aVal+(pVal-50)/8+(cVal-50)/8+(sVal-50)/8;
	WriteProcessMemory(procHandle,(void*)pAddr,&fifty,4,0);
	WriteProcessMemory(procHandle,(void*)sAddr,&fifty,4,0);
	WriteProcessMemory(procHandle,(void*)cAddr,&fifty,4,0);
	WriteProcessMemory(procHandle,(void*)aAddr,&newAttr,4,0);
	CloseHandle(procHandle);
	success=1;
	return "Your attribute points has been reset.\nPlease reload your character to recalculate your HP.";

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,int nShowCmd){
	HWND hwnd;               
	MSG messages;           
	WNDCLASSEX wincl={0};

	g_hInst=hInstance;

	wincl.hInstance=hInstance;
	wincl.lpszClassName=szClassName;
	wincl.lpfnWndProc=WindowProcedure;
	wincl.style=CS_HREDRAW | CS_VREDRAW | CS_OWNDC;; //double clicks CS_DBLCLKS
	wincl.cbSize=sizeof (WNDCLASSEX);

 
	wincl.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wincl.hIconSm=LoadIcon(NULL,IDI_APPLICATION);
	wincl.hCursor=LoadCursor(NULL,IDC_ARROW);
	wincl.lpszMenuName=NULL; 
	wincl.cbClsExtra=0;
	wincl.cbWndExtra=0;
	wincl.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);

	//register the window
	if (!RegisterClassEx (&wincl))
		return 0;

	hwnd=CreateWindowEx (
			0, 
			szClassName,
			"GD Attribute Resetter for B27H2",
			WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
			CW_USEDEFAULT, //default positions
			CW_USEDEFAULT,
			180, //width
			85,  //height
			HWND_DESKTOP,
			NULL,
			hInstance,
			NULL
		);

	ShowWindow (hwnd, SW_SHOW);
	UpdateWindow(hwnd);

    while(GetMessage(&messages,NULL,0,0)){
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }
    return messages.wParam;
}

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
    HWND hwndButton;
    switch (message) {                 /* Handles all Windows Messages */
        case WM_COMMAND:{
              if(((HWND)lParam) && (HIWORD(wParam) == BN_CLICKED)){
                int iMID;
                iMID = LOWORD(wParam);
                switch(iMID){
                  case IDBUTTON:{
						string resetMessage=tryResetAttribute();
						UINT messageType=MB_OK;
						if (!success)
							messageType|=MB_ICONEXCLAMATION;
                       MessageBox(hwnd, (LPCTSTR)resetMessage.c_str(), "Message",messageType);
                       break;
                       }
                  default:
                       break;
                }
              }
              break;
            }
        case WM_DESTROY:{
              PostQuitMessage (0);       /* send a WM_QUIT to Message Queue, to shut off program */
              break;
             }
		        case WM_CREATE:{
               hwndButton = CreateWindowEx(0,                    /* more or ''extended'' styles */
                         TEXT("BUTTON"),                         /* GUI ''class'' to create */
                         TEXT("reset"),                        /* GUI caption */
                         WS_CHILD|WS_VISIBLE|BS_DEFPUSHBUTTON,   /* control styles separated by | */
                         10,                                     /* LEFT POSITION (Position from left) */
                         10,                                     /* TOP POSITION  (Position from Top) */
                         150,                                    /* WIDTH OF CONTROL */
                         30,                                     /* HEIGHT OF CONTROL */
                         hwnd,                                   /* Parent window handle */
                         (HMENU)IDBUTTON,                        /* control''s ID for WM_COMMAND */
                         g_hInst,                                /* application instance */
                         NULL);
               break;
             }
        default:                      /* messages that we will not process */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}