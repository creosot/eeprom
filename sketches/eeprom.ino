#include "avr/eeprom.h"

void saveString(int addr, char *str);
void eewrite(int addr, uint8_t value);
uint8_t eeread(int addr);
uint8_t setLightParamFromEEPROMtoCURRENT(uint8_t row);
uint8_t checkStringLightParamFromINPUT(uint8_t row);
void writeStringLightParamToEEPROM(uint8_t row);
void validateLightParamFromEEPROM();
void checkLightParamFromINPUT();
void printBuffer(char* buf, uint8_t length);
void printBufferEEPROM(const char *buf, uint8_t length);
void printBufferPGM(int adr_ptr);
void printlight_param_TEMP_CURRENT_D_EEPROM();
void setLightParamFromDEFAULTtoEEPROMandCURRENT();
void setLightParamFromCURRENT();
bool readLightSettingFromInput();

#define LENGTH_LIGHT_PARAM_ROW	6
#define COUNT_LIGHT_PARAM_ROW	4
EEMEM const char light_param_EEPROM[COUNT_LIGHT_PARAM_ROW][LENGTH_LIGHT_PARAM_ROW] = { 0 };
EEMEM const char dsa[LENGTH_LIGHT_PARAM_ROW] = { 0 };
const char light_param_DEFAULT[COUNT_LIGHT_PARAM_ROW][LENGTH_LIGHT_PARAM_ROW] = { "20", "25000", "20", "80" };
char light_param_TEMP[COUNT_LIGHT_PARAM_ROW][LENGTH_LIGHT_PARAM_ROW] = { 0 };
long light_param_CURRENT_D[COUNT_LIGHT_PARAM_ROW] = { 0 };
const char string_0[] PROGMEM = "MIN Light";     // "String 0" etc are strings to store - change to suit.
const char string_1[] PROGMEM = "MAX LIGHT";
const char string_2[] PROGMEM = "MIN Led";
const char string_3[] PROGMEM = "MAX Led";
const char * const string_table[COUNT_LIGHT_PARAM_ROW] PROGMEM =   	   // change "string_table" name to suit
{   
	string_0,
	string_1,
	string_2,
	string_3
};

void setup()
{
	Serial.begin(9600);
	Serial.println(F("Start"));
//	saveString((int)light_param_EEPROM[1], (char *)"44");
	Serial.println();
	printBufferEEPROM(light_param_EEPROM[0], sizeof(light_param_EEPROM[0]));
	Serial.println();
	printBufferEEPROM(light_param_EEPROM[1], sizeof(light_param_EEPROM[1]));
	Serial.println();
	printBufferEEPROM(light_param_EEPROM[2], sizeof(light_param_EEPROM[2]));
	Serial.println();
	printBufferEEPROM(light_param_EEPROM[3], sizeof(light_param_EEPROM[3]));
	//	Serial.println();
//	validateLightParamFromEEPROM();
//	readLightSettingFromInput();
//	printlight_param_TEMP_CURRENT_D_EEPROM();
}

void loop()
{
	
	
	
}

void validateLightParamFromEEPROM() 
{
	Serial.println();
	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
	{
		if (setLightParamFromEEPROMtoCURRENT(row)) //проверка епром на цифры, если 0 загрузка в текущий буфер
		{												//если 1 есть не цифра и устанавливаем текущий буфер из дефолта
			Serial.print(row + 1);
			Serial.print(F(": eeprom: "));
			int adr_eeprom = (int)light_param_EEPROM[row];
			char c;
			for (uint8_t i = 0; i < LENGTH_LIGHT_PARAM_ROW; i++)
			{
				c = eeread(adr_eeprom++);
				Serial.print(c);
			}
			Serial.print(F("\n\rEEPROM light param not set or not digit. Set default param:\n\r"));
			setLightParamFromDEFAULTtoEEPROMandCURRENT();
			return;
		}
	}
	Serial.println();
}

uint8_t setLightParamFromEEPROMtoCURRENT(uint8_t row)
{
	int adr_eeprom = (int)light_param_EEPROM[row];
	char *adr_buf = light_param_TEMP[row];
	uint8_t zero = 0;
	char buf[LENGTH_LIGHT_PARAM_ROW];
	for (uint8_t column = 0; column < LENGTH_LIGHT_PARAM_ROW; column++)
	{
		char c = eeread(adr_eeprom);
		if (!c) break;
		if (c < '0' || c > '9')
		{
			return 1;
		}
		*adr_buf = c;
		++adr_eeprom; ++adr_buf; ++zero;
	}
	if (!zero || zero == LENGTH_LIGHT_PARAM_ROW)
	{
		return 1; 
	}
	
	light_param_CURRENT_D[row] = strtol(light_param_TEMP[row], 0, 0);
	Serial.print(row + 1);
	Serial.print(F(": eeprom: "));
	printBufferEEPROM(light_param_EEPROM[row], sizeof(light_param_EEPROM[row]));
	Serial.print("    \tcur_digit: ");
	Serial.println(light_param_CURRENT_D[row]);
	return 0;
}

void setLightParamFromDEFAULTtoEEPROMandCURRENT()
{
	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
	{
		saveString((int)light_param_EEPROM[row], light_param_DEFAULT[row]);
		light_param_CURRENT_D[row] = strtol(light_param_DEFAULT[row], 0, 0);
		
		Serial.print(row + 1);
		Serial.print(F(": default: "));
		printBuffer((char *)light_param_DEFAULT[row], sizeof(light_param_DEFAULT[row]));
		Serial.print(F("   \teeprom: "));
		printBufferEEPROM(light_param_EEPROM[row], sizeof(light_param_EEPROM[row]));
		Serial.print("      \tcur_digit: ");
		Serial.println(light_param_CURRENT_D[row]);
	}
}

void printBufferEEPROM(const char *buf, uint8_t length)
{
	int adr_eeprom = (int)buf;
	char c;
	for (uint8_t i = 0; i < length; i++)
	{
		c = eeread(adr_eeprom++);
		if (!c)
			return;
		Serial.print(c);
	}
}

void printBuffer(char* buf, uint8_t length)
{
	char c;
	for (uint8_t i = 0; i < length; i++)
	{
		c = *buf++;
		if (!c)
			return;
		Serial.print(c);
	}
}

void printlight_param_TEMP_CURRENT_D_EEPROM()
{
	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
	{
		Serial.print(row + 1);
		Serial.print(F(": temp: "));
		char *adr_buf = light_param_TEMP[row];
		printBuffer(adr_buf, sizeof(light_param_TEMP[row]));
		Serial.print("    \tcur_digit: ");
		Serial.print(light_param_CURRENT_D[row]);
		Serial.print(F("    \teeprom: "));
		printBufferEEPROM(light_param_EEPROM[row], sizeof(light_param_EEPROM[row]));
		Serial.println();
	}
}

void printBufferPGM(int adr_ptr) {
	char c;
	uint16_t ptr = pgm_read_word(adr_ptr);      // получаем адрес из таблицы ссылок
	while((c = pgm_read_byte(ptr++)))
	{
		Serial.print(c);         
	}
}


void saveString(int addr, char *str)
{
	while (*str) eewrite(addr++, *str++);
	eewrite(addr, 0);
}

void eewrite(int addr, uint8_t value) 
{
	eeprom_write_byte((unsigned char *) addr, value);
}

uint8_t eeread(int addr)
{
	return eeprom_read_byte((uint8_t *) addr);
}

bool readLightSettingFromInput()
{
	long arr_long[COUNT_LIGHT_PARAM_ROW];
	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
	{
		char *adr_buf = light_param_TEMP[row];
		int adr_eeprom = (int)light_param_EEPROM[row];
		char c;
		uint8_t in_count = 0;
		Serial.print(F("\n\rInput number or Enter to leave the current value\n\r"));
		printBufferPGM((int)&string_table[row]);	
		Serial.print(F(": Current value: "));
		Serial.println(light_param_CURRENT_D[row]);
		Serial.print("input: ");
		while (true)
		{
			if (Serial.available() > 0) {
				c = Serial.read();           // get the character
			
				if((c >= '0') && (c <= '9'))
				{
					if (in_count < LENGTH_LIGHT_PARAM_ROW - 1)
					{
						Serial.print(c);
						*(adr_buf + in_count) = c ;
						++in_count;
					}
					else
					{
						Serial.println(F(" maximum of 5 numbers."));
						Serial.print("input: ");
						for (uint8_t i = 0; i < in_count; i++)
						{
							Serial.print(light_param_TEMP[row][i]);
						}
					}
				}
				else if((c == '\n') || (c == '\r'))
				{
					*(adr_buf + in_count) = '\0';
					break;
				} 
				else
				{
					Serial.print(c);
					Serial.println(F(" Not a number."));
					Serial.print("input: ");
					for (uint8_t i = 0; i < in_count; i++)
					{
						Serial.print(light_param_TEMP[row][i]);
					}
				} 
			}  
		}
		if (!in_count) //ввода не было, ввести текущее значение в буфер
		{
			char c;
			arr_long[row] = light_param_CURRENT_D[row];
			for (uint8_t i = 0; i < LENGTH_LIGHT_PARAM_ROW; i++)
			{
				*(adr_buf + i) = eeread(adr_eeprom + i);
				if (!c) break;
			}	
			Serial.print("\n\rnumber is: ");
			Serial.println(arr_long[row]);
		}
		else
		{
			arr_long[row] = strtol(light_param_TEMP[row], 0, 0);
			Serial.print("\n\rnumber is: ");
			Serial.println(arr_long[row]);
		}
		Serial.println();
	}
	
	if (arr_long[0] > arr_long[1] || arr_long[2] > arr_long[3])
	{
		Serial.println(F("Error MIN value > MAX value.\n\rValues are not valid, enter again"));
		return false;
	}
	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
	{
		char *adr_buf = light_param_TEMP[row];
		int adr_eeprom = (int)light_param_EEPROM[row];
		light_param_CURRENT_D[row] = arr_long[row];
		saveString(adr_eeprom, adr_buf);
	}
	return true;
}