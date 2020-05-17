#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <conio.h>
#include <string>

using namespace std;

int main(){

	char path[] = "C:\\VSrepos\\���\\Lab7\\Client\\Debug\\Client.exe";

	HANDLE hWrite = CreateFile("COM1", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (!hWrite) {
		cout << "Failed to open port!" << endl;
		return 0;
	}

	//������ ��������� ������
	HANDLE readyToWrite = CreateEvent(NULL, TRUE, TRUE, "readyToWrite");
	if (!readyToWrite) {
		cout << "Failed to create readyToWrite Event!" << endl;
		CloseHandle(hWrite);
		return 0;
	}

	//Overlapped ��� ������ �� �����
	HANDLE readyToRead = CreateEvent(NULL, TRUE, FALSE, "readyToRead");
	if (!readyToRead) {
		cout << "Failed to create readyToRead Event!" << endl;
		CloseHandle(hWrite);
		CloseHandle(readyToWrite);
		return 0;
	}
	OVERLAPPED asynchWrite = { 0 };
	asynchWrite.hEvent = readyToRead;

	//����� �������
	STARTUPINFO si;																			//��������� ������������ �������� ������ ��������	
	ZeroMemory(&si, sizeof(STARTUPINFO));													//��������� ��������� si
	si.cb = sizeof(STARTUPINFO);															//������������� ���� cb ��������� si �������� ��������

	PROCESS_INFORMATION client;																//�������� ��������� PROCESS_INFORMATION ��� ������ ��������
	ZeroMemory(&client, sizeof(PROCESS_INFORMATION));										//��������� ��������� pi

	if (!CreateProcess(NULL,																//�������� ������ ��������
		path,
		NULL,
		NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&si,
		&client)) {
		cout << "CreateProcess failed\n";
		return 0;
	}

	string str;
	int i;
	char buf = '\n';
	do {
		i = 0;

		//���� ������ ��� ��������
		cout << "Input message or press 'Enter' to exit" << endl;
		getline(cin, str);

		if (!str.size()) break;

		//������������ �������� ������
		while (i < str.size()) {
			//�������� ��������� ������
			WaitForSingleObject(readyToWrite, INFINITE);							
			ResetEvent(readyToWrite);
			WriteFile(hWrite, &(str[i]), 1, NULL, &asynchWrite);
			i++; 
		}

		//������ '\n'
		//�������� ��������� ������
		WaitForSingleObject(readyToWrite, INFINITE);												
		ResetEvent(readyToWrite);
		WriteFile(hWrite, &buf, 1, NULL, &asynchWrite);

	} while (1);

	//������ 0 ������� - ����� ������
	buf = 0;
	WaitForSingleObject(readyToWrite, INFINITE);
	WriteFile(hWrite, &buf, 1, NULL, &asynchWrite);
	
	//�������� ���������� ������ �������
	WaitForSingleObject(client.hProcess, INFINITE);											
	
	//�������� handl'��
	CloseHandle(client.hProcess);
	CloseHandle(client.hThread);
	CloseHandle(hWrite);
	CloseHandle(readyToRead);
	CloseHandle(readyToWrite);

	return 0;
}