#include "avr/eeprom.h"
#include <BH1750.h>
#include <UIPEthernet.h>

BH1750 lightMeter;
EthernetServer server(23);
EthernetClient client;

void saveString(int addr, char *str);
void eewrite(int addr, uint8_t value);
uint8_t eeread(int addr);
uint8_t setLightParamFromEEPROMtoCURRENT(uint8_t row);
void validateLightParamFromEEPROM();
void printBuffer(char* buf, uint8_t length);
void printBufferToETH(char* buf, uint8_t length);
void printBufferEEPROM(const char *buf, uint8_t length);
void printBufferPGM(int adr_ptr);
void printBufferPGMtoETH(int adr_ptr);
void printlight_param_TEMP_CURRENT_D_EEPROM();
void setLightParamFromDEFAULTtoEEPROMandCURRENT();
bool readLightSettingFromInput();
uint8_t telnet_server();
void display_main_print();
void clearTelnetSymbolBuffer();
void telnet_close_connection();
void display_l();
void l_print();
void display_g();
void g_print();
void default_print();
void printLux();
void lightStream();
void PrintLightParam();
uint8_t validateGIDsFromEEPROM();
void setGIDsToDEFAULT();
void setGIDsFromEEPROM();
void checkGIDs();
void readGIDsFromInput(uint8_t row);
void printGIDA();
void printGIDB();


#define GIDA	0
#define GIDB	1
#define LENGTH_LIGHT_PARAM_ROW	6
#define COUNT_LIGHT_PARAM_ROW	4
#define COUNT_GIDS_ROW	2
#define LENGTH_GIDS_ROW	20
EEMEM const char light_param_EEPROM[COUNT_LIGHT_PARAM_ROW][LENGTH_LIGHT_PARAM_ROW] = { 0 };
EEMEM const char gids_EEPROM[COUNT_GIDS_ROW][LENGTH_GIDS_ROW] = { 0 };
char gids_CURRENT[COUNT_GIDS_ROW][LENGTH_GIDS_ROW] = { 0 };
const char light_param_DEFAULT[COUNT_LIGHT_PARAM_ROW][LENGTH_LIGHT_PARAM_ROW] = { "20", "25000", "20", "80" };
char light_param_TEMP[COUNT_LIGHT_PARAM_ROW][LENGTH_LIGHT_PARAM_ROW] = { 0 };
long light_param_CURRENT_D[COUNT_LIGHT_PARAM_ROW] = { 0 };
const char string_0[] PROGMEM = "MIN Light value:  ";     // "String 0" etc are strings to store - change to suit.
const char string_1[] PROGMEM = "MAX Light value:  ";
const char string_2[] PROGMEM = "MIN Bright value: ";
const char string_3[] PROGMEM = "MAX Bright value: ";
const char * const string_table[COUNT_LIGHT_PARAM_ROW] PROGMEM =   	   // change "string_table" name to suit
{   
	string_0,
	string_1,
	string_2,
	string_3
};
const char gida[] PROGMEM = "GIDA: ";
const char gidb[] PROGMEM = "GIDB: ";
const char * const gids_table[COUNT_GIDS_ROW] PROGMEM =
{   
	gida,
	gidb
};
float lux = -2;
uint8_t mac[6] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };
uint8_t charsReceived = 0;
unsigned long timeOfLastActivity;   //time in milliseconds of last activity
unsigned long allowedConnectTime = 300000;   //five minutes
uint8_t count_s = 0;
#define textBuffSize 4
char textBuff[textBuffSize];

void setup()
{
	Serial.begin(9600);
	Serial.println(F("Start"));
//BH Init
	Wire.begin();
	delay(1000);
	Serial.print(F("\n\rController Init\n\r"));
	delay(1000);
		
	while (1) 
	{
		Serial.print(F("BH Init\n\r"));
		lightMeter.begin();
		delay(500);
		lux = lightMeter.readLightLevel();
		if (lux != -2)
		{
			Serial.println(F("BH1750 Ok"));
			break;
		}
		delay(5000);
	}
//	//Ethernet Init
	while (1) 
	{
		Serial.println(F("Ethernet Init"));
		int ret = Ethernet.begin(mac);
		if (ret != 0)
		{
			Serial.println(F("Ethernet Ok"));
			break;
		}
		Serial.println(F("Not DHCP adress"));
		Serial.print(F("localIP: "));
		Serial.println(Ethernet.localIP());
	}
	Serial.print(F("localIP: "));
	Serial.println(Ethernet.localIP());
	Serial.print(F("subnetMask: "));
	Serial.println(Ethernet.subnetMask());
	Serial.print(F("gatewayIP: "));
	Serial.println(Ethernet.gatewayIP());
	Serial.print(F("dnsServerIP: "));
	Serial.println(Ethernet.dnsServerIP());
	server.begin();
	Serial.print(F("\n\rtelnet server start on 23 port\n\r"));
	
	
	
	//	saveString((int)light_param_EEPROM[1], (char *)"4ff");
	//	Serial.println();
	//	printBufferEEPROM(light_param_EEPROM[0], sizeof(light_param_EEPROM[0]));
	//	Serial.println();
	//	printBufferEEPROM(light_param_EEPROM[1], sizeof(light_param_EEPROM[1]));
	//	Serial.println();
	//	printBufferEEPROM(light_param_EEPROM[2], sizeof(light_param_EEPROM[2]));
	//	Serial.println();
	//	printBufferEEPROM(light_param_EEPROM[3], sizeof(light_param_EEPROM[3]));
		Serial.println();
		printBufferEEPROM(gids_EEPROM[0], LENGTH_GIDS_ROW);
		Serial.println();
		printBufferEEPROM(gids_EEPROM[1], LENGTH_GIDS_ROW);
		Serial.println();
	//	validateLightParamFromEEPROM();
	////	readLightSettingFromInput();
	//	printlight_param_TEMP_CURRENT_D_EEPROM();
	
	checkGIDs();
	printBuffer(gids_CURRENT[0], LENGTH_GIDS_ROW);
	Serial.println();
	printBuffer(gids_CURRENT[1], LENGTH_GIDS_ROW);
}

void loop()
{
	
	telnet_server();
	
}

uint8_t telnet_server()
{
	client = server.available();
	if (client) 
	{
		Serial.println(F("client connected"));
		bool flg_display_main_print = false;
		client.flush();
		timeOfLastActivity = millis();
		while (client.connected())	
		{
			if (!flg_display_main_print)
			{
				display_main_print();
				client.print(F(">>"));
			}
			flg_display_main_print = true;
			
			if (millis() - timeOfLastActivity > allowedConnectTime) {
				client.print(F("\n\rTimeout disconnect.\n\r"));
				telnet_close_connection();
				return 2;
			}
			
			if (client.available()) //Возвращает количество непрочитанных байтов, принятых клиентом от сервера
				{
					textBuff[charsReceived] = (char) client.read();
				
					if (textBuff[2] == 'q')
					{
						textBuff[0] = textBuff[3];
						textBuff[1] = 0;
						textBuff[2] = 0;
						textBuff[3] = 0; 
						charsReceived = 0;
					}
				
					switch (textBuff[0])
					{
					case 'l':
					case 'L':
						display_l(); break;
					case 'g':
					case 'G':
						display_g(); break;
					case 'c':
					case 'C':
						telnet_close_connection(); return 1;
					default:
						default_print(); break;
					}
				}
			if (textBuff[3] == 'l' || textBuff[3] == 'L')
			{
				lightStream(); 
				delay(1000);
			}
		}
	}
	return 0;
}

void display_l()
{
	if (charsReceived == 0)
	{
		charsReceived = 1;
		l_print(); 
	}
	else
	{
		switch (textBuff[1])
		{
		case '1':
			printLux(); l_print(); break;
		case '2':
			l_print(); break;
		case '3':
			PrintLightParam();  l_print(); break;
		case '4':
			readLightSettingFromInput(); l_print(); break;
		case '5':
			textBuff[3] = textBuff[0]; textBuff[0] = 0; charsReceived = 2; break;	
		case 'm':
		case 'M':
			display_main_print(); break;
		default:
			default_print(); break;
		}
	}
}

void l_print()
{
	client.print(F("\n\r1: Light sensor value\n\r2: Bright display\n\r3: MAX MIN parameters\n\r4: Set MAX MIN parameters\n\r5: stream sensor value\n\rm: main display\n\r"));
}

void PrintLightParam()
{
	client.print(F("\n\rCurrent map(function) value:\n\r"));
	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
	{
		printBufferPGMtoETH((int)&string_table[row]);	
		client.println(light_param_CURRENT_D[row]);
	}
}

void display_g()
{
	if (charsReceived == 0)
	{
		charsReceived = 1;
		g_print(); 
	}
	else
	{
		switch (textBuff[1])
		{
		case '1':
			printGIDA(); g_print(); break;
		case '2':
			printGIDB(); g_print(); break;
		case '3':
			readGIDsFromInput(GIDA); g_print(); break;
		case '4':
			readGIDsFromInput(GIDB); g_print(); break;
		case 'm':
		case 'M':
			display_main_print(); break;
		default:
			default_print(); break;
		}
	}
}

void g_print()
{
	client.print(F("\n\r1: Print GIDA: \n\r2: Print GIDB: \n\r3: Set GIDA\n\r4: Set GIDB\n\rm: main display\n\r"));
}

void printGIDA()
{
	client.print(F("GIDA: "));
	client.println(gids_CURRENT[GIDA]);
}

void printGIDB()
{
	client.print(F("GIDB: "));
	client.println(gids_CURRENT[GIDB]);
}

void default_print()
{
	if (textBuff[charsReceived] == '\n' || textBuff[charsReceived] == '\r')
	{
		client.print(F(">")); 
	}
}

void printLux()
{
	lux = lightMeter.readLightLevel();
	if (lux == -2)
	{
		client.print(F("\tLight sensor not answer\n\r"));
	}
	else
	{
		client.print(F("\tLight = "));
		client.println(lux);
	}
}

void lightStream()
{
	printLux();
}

void telnet_close_connection()
{
	client.println(F("\nBye.\n"));
	delay(1000);
	client.stop();
	Serial.println(F("client stop"));
}

void display_main_print()
{
	clearTelnetSymbolBuffer();
	client.print(F("\n\rSymbol and Enter\n\rl: LIGHTs setting\n\rg: GIDs setting\n\rc: Close terminal\n\r"));
}

void clearTelnetSymbolBuffer()
{
	charsReceived = 0;
	for (uint8_t i = 0; i < textBuffSize; i++)
	{
		textBuff[i] = 0;	 
	}
}

void checkGIDs()
{
	if (validateGIDsFromEEPROM()) //return not 0 -gids eeprom установлены, устанавливаем из eeprom
	{
		Serial.println(F("GIDs set to DEFAULT"));
		setGIDsToDEFAULT();
		return;
	}
	//устанвливаем из eeprom
	Serial.println(F("GIDs set from EEPROM"));
	setGIDsFromEEPROM();
}

void setGIDsFromEEPROM()
{
	char c;
	for (uint8_t row = 0; row < COUNT_GIDS_ROW; row++)
	{
		int adr_eeprom = (int)gids_EEPROM[row];
		char *adr_buf = gids_CURRENT[row];
		for (uint8_t i = 0; i < LENGTH_GIDS_ROW; i++)
		{
			c = eeread(adr_eeprom++);
			*adr_buf++ = c;
			if (!c) break;
		}
	}
}

void setGIDsToDEFAULT()
{
	for (uint8_t row = 0; row < COUNT_GIDS_ROW; row++)
	{
		int adr_eeprom = (int)gids_EEPROM[row];
		char *adr_buf = gids_CURRENT[row];
		strcpy(adr_buf, "no set");
		saveString(adr_eeprom, adr_buf);
	}
}

uint8_t validateGIDsFromEEPROM()
{
	Serial.println();
	for (uint8_t row = 0; row < COUNT_GIDS_ROW; row++)
	{
		int adr_eeprom = (int)gids_EEPROM[row];
		char *adr_buf = gids_CURRENT[row];
		uint8_t count_c = 0;
		for (uint8_t column = 0; column < LENGTH_GIDS_ROW; column++)
		{
			char c = eeread(adr_eeprom++);
			if (!c) break; //если конец строки
			if(c == '#' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
			{
				*adr_buf++ = c;
				++count_c;
			}
			else
			{
				Serial.println("not symbol");
				return 1; //нето и несе
			}
		}
		
		if (!count_c || count_c == LENGTH_LIGHT_PARAM_ROW)
		{
			Serial.println("empty");
			return 2; //пусто или нет конца строки
		}
	}	
	return 0;
}

void readGIDsFromInput(uint8_t row)
{
	client.flush();
	char *adr_buf = gids_CURRENT[row];
	int adr_eeprom = (int)gids_EEPROM[row];
	char c;
	uint8_t in_count = 0;
	client.print(F("\n\rInput GID or Enter to leave the current GID\n\r"));
	printBufferPGMtoETH((int)&gids_table[row]);	
	client.print(F(": Current GID: "));
	client.println(gids_CURRENT[row]);
	client.print("input: ");
	while (true)
	{
		if (client.available() > 0) 
		{
			c = client.read();              // get the character
			if(c == '#' || (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
			{
				if (in_count < LENGTH_GIDS_ROW - 1)
				{
					client.print(c);
					*adr_buf++ = c;
					++in_count;
				}
				else
				{
					client.print(F(" maximum of 19 symbols\n\rinput: "));
					printBufferToETH(gids_CURRENT[row], in_count);
				}
			}
			else if((c == '\n') || (c == '\r'))
			{
				client.flush();
				*adr_buf = '\0';
				break;
			} 
			else
			{
				client.print(c);
				client.println(F(" Not a symbol\n\rinput: "));
				printBufferToETH(gids_CURRENT[row], in_count);
			} 
		}  
	}
	
	if (!in_count) //ввода не было, записать gid из EEPROM в буфер
	{
		for (uint8_t i = 0; i < LENGTH_GIDS_ROW; i++)
		{
			c = eeread(adr_eeprom++);
			*adr_buf++ = c;
			if (!c) break;
		}	
	}
	else //ввод есть, записать gid из буфера в EEPROM
	{
		saveString((int)gids_EEPROM[row], gids_CURRENT[row]);
	}
	client.print("\n\rgid is: ");
	client.println(gids_CURRENT[row]);
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
	Serial.println(F("EEPROM-->CURRENT_D"));
	light_param_CURRENT_D[row] = strtol(light_param_TEMP[row], 0, 0);
	Serial.print(row + 1);
	Serial.print(F(": eeprom: "));
	printBufferEEPROM(light_param_EEPROM[row], sizeof(light_param_EEPROM[row]));
	Serial.print("   \tcur_digit: ");
	Serial.println(light_param_CURRENT_D[row]);
	return 0;
}

void setLightParamFromDEFAULTtoEEPROMandCURRENT()
{
	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
	{
		saveString((int)light_param_EEPROM[row], light_param_DEFAULT[row]);
		light_param_CURRENT_D[row] = strtol(light_param_DEFAULT[row], 0, 0);
		Serial.println(F("DFAULT-->EEPROM-CURRENT_D"));
		Serial.print(row + 1);
		Serial.print(F(": default: "));
		printBuffer((char *)light_param_DEFAULT[row], sizeof(light_param_DEFAULT[row]));
		Serial.print(F("   \teeprom: "));
		printBufferEEPROM(light_param_EEPROM[row], sizeof(light_param_EEPROM[row]));
		Serial.print("      \tcur_digit: ");
		Serial.println(light_param_CURRENT_D[row]);
	}
	Serial.println();
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

void printBufferToETH(char* buf, uint8_t length)
{
	char c;
	for (uint8_t i = 0; i < length; i++)
	{
		c = *buf++;
		if (!c)
			return;
		client.print(c);
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
		Serial.print("   \tcur_digit: ");
		Serial.print(light_param_CURRENT_D[row]);
		Serial.print(F("     \teeprom: "));
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

void printBufferPGMtoETH(int adr_ptr) {
	char c;
	uint16_t ptr = pgm_read_word(adr_ptr);       // получаем адрес из таблицы ссылок
	while((c = pgm_read_byte(ptr++)))
	{
		client.print(c);         
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
	client.flush();
	long arr_long[COUNT_LIGHT_PARAM_ROW];
	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
	{
		char *adr_buf = light_param_TEMP[row];
		int adr_eeprom = (int)light_param_EEPROM[row];
		char c;
		uint8_t in_count = 0;
		client.print(F("\n\rInput number or Enter to leave the current value\n\r"));
		printBufferPGMtoETH((int)&string_table[row]);	
		client.print(F(": Current value: "));
		client.println(light_param_CURRENT_D[row]);
		client.print("input: ");
		while (true)
		{
			if (client.available() > 0) {
				c = client.read();             // get the character
			
				if((c >= '0') && (c <= '9'))
				{
					if (in_count < LENGTH_LIGHT_PARAM_ROW - 1)
					{
						client.print(c);
						*(adr_buf + in_count) = c;
						++in_count;
					}
					else
					{
						client.println(F(" maximum of 5 numbers."));
						client.print("input: ");
						for (uint8_t i = 0; i < in_count; i++)
						{
							client.print(light_param_TEMP[row][i]);
						}
					}
				}
				else if((c == '\n') || (c == '\r'))
				{
					client.flush();
					*(adr_buf + in_count) = '\0';
					break;
				} 
				else
				{
					client.print(c);
					client.println(F(" Not a number."));
					client.print("input: ");
					for (uint8_t i = 0; i < in_count; i++)
					{
						client.print(light_param_TEMP[row][i]);
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
					c = eeread(adr_eeprom + i);
					*(adr_buf + i) = c;
					if (!c) break;
				}	
				client.print("\n\rnumber is: ");
				client.println(arr_long[row]);
			}
		else
		{
			arr_long[row] = strtol(light_param_TEMP[row], 0, 0);
			client.print("\n\rnumber is: ");
			client.println(arr_long[row]);
		}
		client.println();
	}
	
	if (arr_long[0] > arr_long[1] || arr_long[2] > arr_long[3])
	{
		client.println(F("Error MIN value > MAX value.\n\rValues are not valid, enter again"));
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

//bool readLightSettingFromInput()
//{
//	long arr_long[COUNT_LIGHT_PARAM_ROW];
//	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
//	{
//		char *adr_buf = light_param_TEMP[row];
//		int adr_eeprom = (int)light_param_EEPROM[row];
//		char c;
//		uint8_t in_count = 0;
//		Serial.print(F("\n\rInput number or Enter to leave the current value\n\r"));
//		printBufferPGM((int)&string_table[row]);	
//		Serial.print(F(": Current value: "));
//		Serial.println(light_param_CURRENT_D[row]);
//		Serial.print("input: ");
//		while (true)
//		{
//			if (Serial.available() > 0) {
//				c = Serial.read();           // get the character
//			
//				if((c >= '0') && (c <= '9'))
//				{
//					if (in_count < LENGTH_LIGHT_PARAM_ROW - 1)
//					{
//						Serial.print(c);
//						*(adr_buf + in_count) = c ;
//						++in_count;
//					}
//					else
//					{
//						Serial.println(F(" maximum of 5 numbers."));
//						Serial.print("input: ");
//						for (uint8_t i = 0; i < in_count; i++)
//						{
//							Serial.print(light_param_TEMP[row][i]);
//						}
//					}
//				}
//				else if((c == '\n') || (c == '\r'))
//				{
//					*(adr_buf + in_count) = '\0';
//					break;
//				} 
//				else
//				{
//					Serial.print(c);
//					Serial.println(F(" Not a number."));
//					Serial.print("input: ");
//					for (uint8_t i = 0; i < in_count; i++)
//					{
//						Serial.print(light_param_TEMP[row][i]);
//					}
//				} 
//			}  
//		}
//		if (!in_count) //ввода не было, ввести текущее значение в буфер
//		{
//			char c;
//			arr_long[row] = light_param_CURRENT_D[row];
//			for (uint8_t i = 0; i < LENGTH_LIGHT_PARAM_ROW; i++)
//			{
//				*(adr_buf + i) = eeread(adr_eeprom + i);
//				if (!c) break;
//			}	
//			Serial.print("\n\rnumber is: ");
//			Serial.println(arr_long[row]);
//		}
//		else
//		{
//			arr_long[row] = strtol(light_param_TEMP[row], 0, 0);
//			Serial.print("\n\rnumber is: ");
//			Serial.println(arr_long[row]);
//		}
//		Serial.println();
//	}
//	
//	if (arr_long[0] > arr_long[1] || arr_long[2] > arr_long[3])
//	{
//		Serial.println(F("Error MIN value > MAX value.\n\rValues are not valid, enter again"));
//		return false;
//	}
//	for (uint8_t row = 0; row < COUNT_LIGHT_PARAM_ROW; row++)
//	{
//		char *adr_buf = light_param_TEMP[row];
//		int adr_eeprom = (int)light_param_EEPROM[row];
//		light_param_CURRENT_D[row] = arr_long[row];
//		saveString(adr_eeprom, adr_buf);
//	}
//	return true;
//}