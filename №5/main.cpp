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
00h � �������
	01h � ������� ����������
02h � ������
	03h � ������ ����������
04h � ����
	05h � ���� ����������
07h � ���� ������
08h � �����
09h � ��� ��� ������� ��� �����
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

	delayCounter--;																		//�������� �������
	oldTimer();																			//������ ����������
}

void interrupt newAlarm(...){
	oldAlarm();																			//������ ����������
	resetAlarm();
	puts("Alarm");
}

int main(){

	int f = 1;

	_disable();

	oldAlarm = getvect(0x4A);															//�������� ������ ���������� ����������

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

	//������ �������� �� ��������� RTC
	for (int i = 0; i < 6; i++) {
		outp(0x70, registers[i]);
		date[i] = inp(0x71);
	}

	//����� ������� � ����
	printf("%x:%02x:%02x  %x %s %x\n", date[2], date[1], date[0], date[3], month[date[4]-1], date[5]);
}

void setTime() {
	
	int i;

	inputTime();
	
	_disable();																			//��������� ����������

	outp(0x70, 0xA);																	//������� ������� � ��� 0�71 �����

	//�������� ��������� ���������� �����
	for (i = 0; i < 10; i++) {

		if ((inp(0x71) & 0x80) == 0)													//�������� 7 ����
			break;
	}

	//�� ������� ��������� ������������ �����
	if (i == 10) {
		printf("Failed to wait for the RTC release\n\n");
		return;
	}

	outp(0x70, 0xB);																	//������� ������� � ��� 0�71 �����
	outp(0x71, inp(0x71) | 0x80);														//��������� ���������� ���� ���������� �����

	//������� ����� � ��������������� ��������
	for (i = 0; i < 3; i++) {
	
		outp(0x70, registers[i]);
		outp(0x71, newTime[i]);	
	}

	outp(0x70, 0xB);																	//������� ������� � ��� 0�71 �����
	outp(0x71, inp(0x71) & 0x7F);														//�������� ���������� ���� ���������� �����

	_enable();																			//��������� ����������
	system("cls");
}

void setDelay() {

	fflush(stdin);
	printf("Input delay in milliseconds: ");
	scanf("%u", &delayCounter);
	
	printf("Delay start time : ");
	getTime();

	_disable();																			//��������� ����������
	oldTimer = getvect(0x70);															//�������� ������ ���������� ����������
	setvect(0x70, newTimer);															//���������� ����� ���������� ����������
	_enable();																			//��������� ����������

	outp(0xA1, inp(0xA1) & 0xFE);														//�������������� ����� ������� ������� �� RTC

	outp(0x70, 0xB);																	//������� ������� � ��� 0�71 �����
	outp(0x71, inp(0x71) | 0x40);														//�������� �������������� ����������

	while (delayCounter) {}																//��������� ��������� delay

	printf("Delay end time   : ");
	getTime();

	_disable();																			//��������� ����������
	setvect(0x70, oldTimer);															//���������� ������ ���������� ����������
	_enable();																			//��������� ����������
}

void setAlarm() {
	
	int i;

	inputTime();

	_disable();																			//��������� ����������

	outp(0x70, 0xA);																	//������� ������� � ��� 0�71 �����

	//�������� ��������� ���������� �����
	for (i = 0; i < 10; i++) {

		if ((inp(0x71) & 0x80) == 0)													//�������� 7 ����
			break;
	}

	//�� ������� ��������� ������������ �����
	if (i == 10) {
		printf("Failed to wait for the RTC release\n\n");
		return;
	}

	//������� ����� � ��������������� ��������
	for (i = 0; i < 3; i++) {

		outp(0x70, alarmRegisters[i]);
		outp(0x71, newTime[i]);
	}

	outp(0x70, 0xB);																	//������� ������� � ��� 0�71 �����
	outp(0x71, (inp(0x71) | 0x20));														//���������� ����������� ���������� (���������)

	outp(0xA1, inp(0xA1) & 0xFE);														//�������������� ����� ������� ������� �� RTC

	
	setvect(0x4A, newAlarm);															//���������� ����� ���������� ����������
	
	_enable();																			//��������� ����������
}

void resetAlarm() {

	//��������� ��� �� ��� ����������
	if (oldAlarm == NULL){
		return;
	}

	int i;

	_disable();

	outp(0x70, 0xA);																	//������� ������� � ��� 0�71 �����

	//�������� ��������� ���������� �����
	for (i = 0; i < 10; i++) {

		if ((inp(0x71) & 0x80) == 0)													//�������� 7 ����
			break;
	}

	//�� ������� ��������� ������������ �����
	if (i == 100) {
		printf("Failed to wait for the RTC release\n\n");
		return;
	}

	outp(0x70, 0xB);																	//������� ������� � ��� 0�71 �����
	outp(0x71, (inp(0x71) & 0xDF));														//������ ����������� ���������� (���������)

	setvect(0x4A, oldAlarm);															//���������� ������ ���������� ����������
	outp(0xA1, (inp(0xA0) | 0x01));														//������������� ����� ������� ������� �� RTC

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