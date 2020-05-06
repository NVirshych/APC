#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>

unsigned int delayCounter = 0;

char newTime[3];

unsigned int registers[] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09 };
unsigned int alarmRegisters[] = { 0x01, 0x03, 0x05 };

/*
00h – секунды
	01h – секунды будильника
02h – минуты
	03h – минуты будильника
04h – часы
	05h – часы будильника
07h – день месяца
08h – месяц
09h – год две младшие две цифры
*/

void interrupt(*oldTimer)(...);
void interrupt(*oldAlarm) (...);

void getTime();
void setTime();
void setDelay();
void setAlarm();
void resetAlarm();
void inputTime();
char getBCD(int);

void interrupt newTimer(...){

	delayCounter--;																		//Уменьшит счетчик
	oldTimer();																			//Старое прерывание
}

void interrupt newAlarm(...){
	oldAlarm();																			//Старое прерывание
	resetAlarm();
	puts("Alarm");
}

int main(){

	int f = 1;

	_disable();

	oldAlarm = getvect(0x4A);															//Получить старый обработчик прерывания

	_enable();

	while (f){

		printf("1 - Current time\n");
		printf("2 - Set time\n");
		printf("3 - Set delay\n");
		printf("4 - Set alarm\n");
		printf("5 - Exit\n");

		switch (getch()){
		case '1':{
			system("cls");
			getTime();
			break;
		}
		case '2':{
			system("cls");
			setTime();
			break;
		}
		case '3':{
			system("cls");
			setDelay();
			break;
		}
		case '4':{
			system("cls");
			setAlarm();
			break;
		}
		case '5': {
			f = 0;
			break;
		}
		default:{
			system("cls");
			break;
		}
		}
	}
	return 0;
}

void getTime() {
	
	int date[6];
	char month[][10] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

	//Чтение значений из регистров RTC
	for (int i = 0; i < 6; i++) {
		outp(0x70, registers[i]);
		date[i] = inp(0x71);
	}

	//Вывод времени и даты
	printf("%x:%02x:%02x  %x %s %x\n", date[2], date[1], date[0], date[3], month[date[4]-1], date[5]);
}

void setTime() {
	
	int i;

	inputTime();
	
	_disable();																			//Запретить прерывания

	outp(0x70, 0xA);																	//Выбрать регистр А для 0х71 порта

	//Ожидание окончания обновления часов
	for (i = 0; i < 10; i++) {

		if ((inp(0x71) & 0x80) == 0)													//Проверка 7 бита
			break;
	}

	//Не удалось дождаться освобождения часов
	if (i == 10) {
		printf("Failed to wait for the RTC release\n\n");
		return;
	}

	outp(0x70, 0xB);																	//Выбрать регистр В для 0х71 порта
	outp(0x71, inp(0x71) | 0x80);														//Отключить внутренний цикл обновления часов

	//Занести время в соответствующие регистры
	for (i = 0; i < 3; i++) {
	
		outp(0x70, registers[i]);
		outp(0x71, newTime[i]);	
	}

	outp(0x70, 0xB);																	//Выбрать регистр В для 0х71 порта
	outp(0x71, inp(0x71) & 0x7F);														//Включить внутренний цикл обновления часов

	_enable();																			//Разрешить прерывания
	system("cls");
}

void setDelay() {

	fflush(stdin);
	printf("Input delay in milliseconds: ");
	scanf("%u", &delayCounter);
	
	printf("Delay start time : ");
	getTime();

	_disable();																			//Запретить прерывания
	oldTimer = getvect(0x70);															//Получить старый обработчик прерывания
	setvect(0x70, newTimer);															//Установить новый обработчик прерывания
	_enable();																			//Разрешить прерывания

	outp(0xA1, inp(0xA1) & 0xFE);														//Размаскировать линии сигнала запроса от RTC

	outp(0x70, 0xB);																	//Выбрать регистр В для 0х71 порта
	outp(0x71, inp(0x71) | 0x40);														//Включить прериодические прерывания

	while (delayCounter) {}																//Подождать обнуления delay

	printf("Delay end time   : ");
	getTime();

	_disable();																			//Запретить прерывания
	setvect(0x70, oldTimer);															//Установить старый обработчик прерывания
	_enable();																			//Разрешить прерывания
}

void setAlarm() {
	
	int i;

	inputTime();

	_disable();																			//Запретить прерывания

	outp(0x70, 0xA);																	//Выбрать регистр А для 0х71 порта

	//Ожидание окончания обновления часов
	for (i = 0; i < 10; i++) {

		if ((inp(0x71) & 0x80) == 0)													//Проверка 7 бита
			break;
	}

	//Не удалось дождаться освобождения часов
	if (i == 10) {
		printf("Failed to wait for the RTC release\n\n");
		return;
	}

	//Занести время в соответствующие регистры
	for (i = 0; i < 3; i++) {

		outp(0x70, alarmRegisters[i]);
		outp(0x71, newTime[i]);
	}

	outp(0x70, 0xB);																	//Выбрать регистр В для 0х71 порта
	outp(0x71, (inp(0x71) | 0x20));														//Разрешение сигнального прерывания (будильник)

	outp(0xA1, inp(0xA1) & 0xFE);														//Размаскировать линии сигнала запроса от RTC

	
	setvect(0x4A, newAlarm);															//Установить новый обработчик прерывания
	
	_enable();																			//Разрешить прерывания
}

void resetAlarm() {

	//Будильник еще не был установлен
	if (oldAlarm == NULL){
		return;
	}

	int i;

	_disable();

	outp(0x70, 0xA);																	//Выбрать регистр А для 0х71 порта

	//Ожидание окончания обновления часов
	for (i = 0; i < 10; i++) {

		if ((inp(0x71) & 0x80) == 0)													//Проверка 7 бита
			break;
	}

	//Не удалось дождаться освобождения часов
	if (i == 100) {
		printf("Failed to wait for the RTC release\n\n");
		return;
	}

	outp(0x70, 0xB);																	//Выбрать регистр В для 0х71 порта
	outp(0x71, (inp(0x71) & 0xDF));														//Запрет сигнального прерывания (будильник)

	setvect(0x4A, oldAlarm);															//Установить старый обработчик прерывания
	outp(0xA1, (inp(0xA0) | 0x01));														//Замаскировать линии сигнала запроса от RTC

	_enable();
}

void inputTime() {
	int num;

	do {
		fflush(stdin);
		printf("Input hours: ");
		scanf("%d", &num);
	} while (num > 23 || num < 0);
	newTime[2] = getBCD(num);

	do {
		fflush(stdin);
		printf("Input minutes: ");
		scanf("%d", &num);
	} while (num > 59 || num < 0);
	newTime[1] = getBCD(num);

	do {
		fflush(stdin);
		printf("Input seconds: ");
		scanf("%d", &num);
	} while (num > 59 || num < 0);
	newTime[0] = getBCD(num);
}

char getBCD(int dec) {
	return (dec / 10 * 16) + (dec % 10);
}