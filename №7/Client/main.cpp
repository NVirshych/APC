#include <Windows.h>
#include <conio.h>
#include <iostream>

using namespace std;

int main() {

	char buf;		//����� ��� ������
	int f = 1;

	HANDLE hRead = CreateFile("COM2", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (!hRead) {
		cout << "Failed to open port!" << endl;
		return 0;
	}

	//������ �������� ������������ ������
	HANDLE finishedReading = CreateEvent(NULL, TRUE, FALSE, "finishedReading");
	if (!finishedReading) {
		cout << "Failed to create readyToWrite Event!" << endl;
		CloseHandle(hRead);
		return 0;
	}
	//Overlapped ��� ������ �� �����
	OVERLAPPED asynchRead = { 0 };
	asynchRead.hEvent = finishedReading;

	//������ ��������� ������
	HANDLE readyToWrite = OpenEvent(EVENT_ALL_ACCESS, FALSE, "readyToWrite");
	if (!readyToWrite) {
		cout << "Failed to open readyToWrite Event!" << endl;
		CloseHandle(hRead);
		CloseHandle(finishedReading);
		return 0;
	}

	//������ ��������� ������
	HANDLE readyToRead = OpenEvent(EVENT_ALL_ACCESS, FALSE, "readyToRead");
	if (!readyToRead) {
		cout << "Failed to open readyToRead Event!" << endl;
		CloseHandle(hRead);
		CloseHandle(finishedReading);
		CloseHandle(readyToWrite);
		return 0;
	}

	do {

		//�������� ��������� ������
		WaitForSingleObject(readyToRead, INFINITE);			
		ResetEvent(readyToRead);

		ReadFile(hRead, &buf, 1, NULL, &asynchRead);
		//�������� ��������� ������
		WaitForSingleObject(finishedReading, INFINITE);						
		SetEvent(readyToWrite);

		//����� �������
		if (!buf) break;

		if (f) {
			cout << "New message: ";
			f = 0;
		}

		cout << buf;	

		if (buf == '\n')
			f = 1;

	} while (1);

	//�������� handl'��
	CloseHandle(hRead);
	CloseHandle(readyToRead);
	CloseHandle(readyToWrite);
	CloseHandle(finishedReading);

	return 0;
}