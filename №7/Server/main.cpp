#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <conio.h>
#include <string>

using namespace std;

int main(){

	char path[] = "C:\\VSrepos\\АПК\\Lab7\\Client\\Debug\\Client.exe";

	HANDLE hWrite = CreateFile("COM1", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (!hWrite) {
		cout << "Failed to open port!" << endl;
		return 0;
	}

	//Сигнал окончания чтения
	HANDLE readyToWrite = CreateEvent(NULL, TRUE, TRUE, "readyToWrite");
	if (!readyToWrite) {
		cout << "Failed to create readyToWrite Event!" << endl;
		CloseHandle(hWrite);
		return 0;
	}

	//Overlapped для чтения из порта
	HANDLE readyToRead = CreateEvent(NULL, TRUE, FALSE, "readyToRead");
	if (!readyToRead) {
		cout << "Failed to create readyToRead Event!" << endl;
		CloseHandle(hWrite);
		CloseHandle(readyToWrite);
		return 0;
	}
	OVERLAPPED asynchWrite = { 0 };
	asynchWrite.hEvent = readyToRead;

	//Новый процесс
	STARTUPINFO si;																			//Структура определяющая свойства нового процесса	
	ZeroMemory(&si, sizeof(STARTUPINFO));													//Обнуление структуры si
	si.cb = sizeof(STARTUPINFO);															//Инициализация поля cb структуры si размером структры

	PROCESS_INFORMATION client;																//Создание структуры PROCESS_INFORMATION для нового процесса
	ZeroMemory(&client, sizeof(PROCESS_INFORMATION));										//Обнуление структуры pi

	if (!CreateProcess(NULL,																//Создание нового процесса
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

		//Ввод строки для передачи
		cout << "Input message or press 'Enter' to exit" << endl;
		getline(cin, str);

		if (!str.size()) break;

		//Посимвольная передача строки
		while (i < str.size()) {
			//Ожидание окончания чтения
			WaitForSingleObject(readyToWrite, INFINITE);							
			ResetEvent(readyToWrite);
			WriteFile(hWrite, &(str[i]), 1, NULL, &asynchWrite);
			i++; 
		}

		//Запись '\n'
		//Ожидание окончания чтения
		WaitForSingleObject(readyToWrite, INFINITE);												
		ResetEvent(readyToWrite);
		WriteFile(hWrite, &buf, 1, NULL, &asynchWrite);

	} while (1);

	//Запись 0 символа - конец работы
	buf = 0;
	WaitForSingleObject(readyToWrite, INFINITE);
	WriteFile(hWrite, &buf, 1, NULL, &asynchWrite);
	
	//Ожидание завершения работы клиента
	WaitForSingleObject(client.hProcess, INFINITE);											
	
	//Закрытие handl'ов
	CloseHandle(client.hProcess);
	CloseHandle(client.hThread);
	CloseHandle(hWrite);
	CloseHandle(readyToRead);
	CloseHandle(readyToWrite);

	return 0;
}