#include <iostream>
#include <windows.h>
#include <string>
#include <ctime>
#include <tchar.h>
#include <TlHelp32.h> 
#include <Psapi.h>
#include <wchar.h>
#include <cstdint>
#pragma comment( lib, "psapi" )

using namespace std;

string curStatus;

int gameSet;
int updateOnNext;

DWORD ironbitsOffsets[]={0x68,0x560,0x6C,0x28,0x7F8};

DWORD testOffsets[]={0x68,0,0x8,0x8,0x0};

DWORD physiqueOffsets[]={0x68,0x964,0x0,0x8,0x0};
DWORD cunningOffsets[]={0x68,0x964,0x8,0x8,0x0};
DWORD spiritOffsets[]={0x68,0x964,0x4,0x8,0x0};
DWORD attributeOffsets[]={0x68,0xdac};


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
		cout<<"failed to read memory "<<dec<<GetLastError()<<endl;
		//exit(0);
		return 0;
	}

	for (int i=0;i<offsetSize;i++){
		ptr=memPtr+offsets[i];
		//cout<<"Accessing: "<<hex<<ptr<<endl;
		if (!ReadProcessMemory(procHandle,(void*)ptr,&memPtr,4,NULL))
			return 0;
		//cout<<"Address: "<<hex<<memPtr<<endl;
	}
	return ptr;
}

int main(){
	HWND hGameWindow = NULL;
	DWORD procID= NULL;
	HANDLE procHandle=NULL;
	int ready=false;
	int prevStat=false;
	int updateText=1;

	while(1){
		gameSet=0;
		ready=0;
		hGameWindow=FindWindow(NULL,"Grim Dawn");
		if (hGameWindow){
			GetWindowThreadProcessId(hGameWindow,&procID);
			if (procID){
				procHandle=OpenProcess(PROCESS_ALL_ACCESS,false,procID);
				if (procHandle==INVALID_HANDLE_VALUE||!procHandle)
					curStatus="Failed to open process handle.";
				else{
					curStatus="Grim Dawn process found.";
					ready=1;
				}
			}
			else
				curStatus="Failed to find process ID.";

		}
		else
			curStatus="Grim Dawn not running.";

		
		//system("cls");
		if (prevStat!=ready){
			if (!ready)
				cout<<curStatus<<endl;
			else{
				cout<<"Grim Dawn Detected. Make sure you have logged in your character."<<endl;
				cout<<"Press F1 to reset your attributes."<<endl;
			}
		}

		prevStat=ready;


		if (GetAsyncKeyState(VK_F1)){
			if (ready){
				float pVal;
				float cVal;
				float sVal;
				int aVal;
				DWORD pAddr=getFinalAddress(procID,procHandle,0x230230,physiqueOffsets,5);
				DWORD cAddr=getFinalAddress(procID,procHandle,0x230230,cunningOffsets,5);
				DWORD sAddr=getFinalAddress(procID,procHandle,0x230230,spiritOffsets,5);
				DWORD aAddr=getFinalAddress(procID,procHandle,0x230230,attributeOffsets,2);
				ReadProcessMemory(procHandle,(void*)pAddr,&pVal,sizeof(pVal),0);
				ReadProcessMemory(procHandle,(void*)cAddr,&cVal,sizeof(cVal),0);
				ReadProcessMemory(procHandle,(void*)sAddr,&sVal,sizeof(sVal),0);
				ReadProcessMemory(procHandle,(void*)aAddr,&aVal,sizeof(aVal),0);
				cout<<"Current available points: "<<aVal<<endl;
				cout<<"Current physique: "<<pVal<<endl;
				cout<<"Current cunning: "<<cVal<<endl;
				cout<<"Current spirit: "<<sVal<<endl;
				if (int(pVal-50)%8||int(cVal-50)%8||int(sVal-50)%8){
					cout<<"One of your attribute is abnormal. You probably cheated."<<endl;
				}
				else{
					float fifty=50;
					int newAttr=aVal+(pVal-50)/8+(cVal-50)/8+(sVal-50)/8;
					WriteProcessMemory(procHandle,(void*)pAddr,&fifty,4,0);
					WriteProcessMemory(procHandle,(void*)sAddr,&fifty,4,0);
					WriteProcessMemory(procHandle,(void*)cAddr,&fifty,4,0);
					WriteProcessMemory(procHandle,(void*)aAddr,&newAttr,4,0);
					cout<<"Your attribute points has been reset."<<endl;
					cout<<"Please reload your character to recalculate your HP."<<endl;
				}
			}
		}
		Sleep(100);
		
	}

	CloseHandle(procHandle);

	return 0;
}

