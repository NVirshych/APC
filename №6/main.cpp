#include <dos.h>
#include <stdio.h>

typedef int bool;

#define true 1
#define false 0
#define ITER 10

struct VIDEO
{
	unsigned char symb;				//Отображаемый символ
	unsigned char attr;				//Цвет
};


void interrupt(*oldKeyboard)(...);

void print(int);					//Вывести код возврата в резидентной программе
void blink();						//Моргнуть индикаторами
bool dataRegFree();					//Проверяем свободен ли регистр данных
bool retCodeGood();					//Проверка кода возврата
bool writeAndCheck(int);				//Записать в регистр данных значение и проверить результат обработки


void interrupt newKeyboard(...) {

	print(inp(0x60));
	oldKeyboard();					//Старое прерывание
}

void print(int val) {

	char str[] = "Return code : 0x";
	char sym;								//Символ для вывода на экран
	int tmp;
	int i;
	unsigned char attr = 0x5E;						//Цвет

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

	//Управляющий байт
	if (!writeAndCheck(0xED))
		return;

	//Включить все индикаторы
	if (!writeAndCheck(0xFF))
		return;

	//Подождать
	delay(2000);

	//Управляющий байт
	if (!writeAndCheck(0xED))
		return;

	//Выключить все индикаторы
	if (!writeAndCheck(0x00))
		return;

	printf("Finished blinking\n");	
}

//Проверяем свободен ли регистр данных
bool dataRegFree() {

	for (int i = 0; i < ITER; i++) {

		//Проверяем 1 бит регистра состояния
		if (!(inp(0x64) & 0x02))
			return true;
	}
	printf("Keyboard data register is busy! Please try again later.\n");
	return false;
}

//Проверка кода возврата
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

//Записать в регистр данных значение и проверить результат обработки
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

	//Переопределение прерывания клавиатуры
	_disable();
	oldKeyboard = getvect(0x09);
	setvect(0x09, newKeyboard);
	_enable();

	FP_SEG(fp) = _psp;							//Префикс сегмента программы
	FP_OFF(fp) = 0x2c;							//Смещение сегмента данных с переменными среды
	_dos_freemem(*fp);							//Освобождение памяти

	_dos_keep(0, (_DS - _CS) + (_SP / 16) + 1);				//Оставляем программу резидентной

	return 0;
}
