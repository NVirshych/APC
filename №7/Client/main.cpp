#include <Windows.h>
#include <conio.h>
#include <iostream>

using namespace std;

int main() {

	char buf;		//Буфер для записи
	int f = 1;

	HANDLE hRead = CreateFile("COM2", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (!hRead) {
		cout << "Failed to open port!" << endl;
		return 0;
	}

	//Сигнал окочания асинхронного чтения
	HANDLE finishedReading = CreateEvent(NULL, TRUE, FALSE, "finishedReading");
	if (!finishedReading) {
		cout << "Failed to create readyToWrite Event!" << endl;
		CloseHandle(hRead);
		return 0;
	}
	//Overlapped для чтения из порта
	OVERLAPPED asynchRead = { 0 };
	asynchRead.hEvent = finishedReading;

	//Сигнал окончания чтения
	HANDLE readyToWrite = OpenEvent(EVENT_ALL_ACCESS, FALSE, "readyToWrite");
	if (!readyToWrite) {
		cout << "Failed to open readyToWrite Event!" << endl;
		CloseHandle(hRead);
		CloseHandle(finishedReading);
		return 0;
	}

	//Сигнал окончания записи
	HANDLE readyToRead = OpenEvent(EVENT_ALL_ACCESS, FALSE, "readyToRead");
	if (!readyToRead) {
		cout << "Failed to open readyToRead Event!" << endl;
		CloseHandle(hRead);
		CloseHandle(finishedReading);
		CloseHandle(readyToWrite);
		return 0;
	}

	do {

		//Ожидание окончания записи
		WaitForSingleObject(readyToRead, INFINITE);			
		ResetEvent(readyToRead);

		ReadFile(hRead, &buf, 1, NULL, &asynchRead);
		//Ожидание окончания чтения
		WaitForSingleObject(finishedReading, INFINITE);						
		SetEvent(readyToWrite);

		//Вывод символа
		if (!buf) break;

		if (f) {
			cout << "New message: ";
			f = 0;
		}

		cout << buf;	

		if (buf == '\n')
			f = 1;

	} while (1);

	//Закрытие handl'ов
	CloseHandle(hRead);
	CloseHandle(readyToRead);
	CloseHandle(readyToWrite);
	CloseHandle(finishedReading);

	return 0;
}