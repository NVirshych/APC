#include <dos.h>
#include <stdio.h>

typedef int bool;

#define true 1
#define false 0
#define ITER 10

struct VIDEO
{
	unsigned char symb;															//������������ ������
	unsigned char attr;															//����
};


void interrupt(*oldKeyboard)(...);

void print(int);																//������� ��� �������� � ����������� ���������
void blink();																	//�������� ������������
bool dataRegFree();																//��������� �������� �� ������� ������
bool retCodeGood();																//�������� ���� ��������
bool writeAndCheck(int);														//�������� � ������� ������ �������� � ��������� ��������� ���������


void interrupt newKeyboard(...) {

	print(inp(0x60));
	oldKeyboard();																//������ ����������
}

void print(int val) {

	char str[] = "Return code : 0x";
	char sym;																	//������ ��� ������ �� �����
	int tmp;
	int i;
	unsigned char attr = 0x5E;													//����

	VIDEO far* screen = (VIDEO far*)MK_FP(0xB800, 0) + 60;

	for (i = 0; i < 16; i++) {

		screen->symb = str[i];
		screen->attr = attr;
		screen++;
	}

	screen++;

	for (i = 1; i < 3; i++) {
	
		tmp = val % 16;
	
		switch (tmp) {
		
		case(15): {
			sym = 'F';
			break;
		}
		case(14): {
			sym = 'E';
			break;
		}
		case(13): {
			sym = 'D';
			break;
		}
		case(12): {
			sym = 'C';
			break;
		}
		case(11): {
			sym = 'B';
			break;
		}
		case(10): {
			sym = 'A';
			break;	
		}
		default:
			sym = tmp + '0';
		}
	
		screen->symb = sym;
		screen->attr = attr;
		screen--;
		val /= 16;
	}
}

void blink() {

	if (!dataRegFree())
		return;

	//����������� ����
	if (!writeAndCheck(0xED))
		return;

	//�������� ��� ����������
	if (!writeAndCheck(0xFF))
		return;

	//���������
	delay(2000);

	//����������� ����
	if (!writeAndCheck(0xED))
		return;

	//��������� ��� ����������
	if (!writeAndCheck(0x00))
		return;

	printf("Finished blinking\n");	
}

//��������� �������� �� ������� ������
bool dataRegFree() {

	for (int i = 0; i < ITER; i++) {

		//��������� 1 ��� �������� ���������
		if (!(inp(0x64) & 0x02))
			return true;
	}
	printf("Keyboard data register is busy! Please try again later.\n");
	return false;
}

//�������� ���� ��������
bool retCodeGood() {

	for (int i = 0; i < 3; i++) {

		int val = inp(0x60);
		printf("Return code : 0x%02X ", val);

		if (val == 0xFE) {
			printf(" - An error occured while processing data byte.\n");
		}
		else {
			printf(" - So far so good.\n");
			return true;
		}
	}
	return false;
}

//�������� � ������� ������ �������� � ��������� ��������� ���������
bool writeAndCheck(int val) {

	outp(0x60, val);

	if (!dataRegFree())
		return false;

	if (!retCodeGood())
		return false;

	return true;
}

int main() {

	unsigned far* fp;

	blink();

	//��������������� ���������� ����������
	_disable();
	oldKeyboard = getvect(0x09);
	setvect(0x09, newKeyboard);
	_enable();

	FP_SEG(fp) = _psp;															//������� �������� ���������
	FP_OFF(fp) = 0x2c;															//�������� �������� ������ � ����������� �����
	_dos_freemem(*fp);															//������������ ������

	_dos_keep(0, (_DS - _CS) + (_SP / 16) + 1);									//��������� ��������� �����������

	return 0;
}