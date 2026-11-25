/*
 * screen3.c
 *
 *  Created on: 20 авг. 2025 г.
 *      Author: Rafael Boltachev
 */


#include "screen4.h"
#include "screens.h"
#include "str_util.h"
#include "mki_data.h"
#include "display.h"

char setevoe_upravlenie_addr_pu[4] = {'\0'};

//--------------- ВВОД АДРЕСА ПУ --------------------

void screen4_paint(void)
{
	clear_display();
	set_display(0, 0, "ВВЕДИТЕ АДРЕС ПУ");
	set_display(2, 0, setevoe_upravlenie_addr_pu);
	send_display();
}

void screen4_init(void)
{
	memset(setevoe_upravlenie_addr_pu, 0, sizeof(setevoe_upravlenie_addr_pu));
}

void screen4_button_press(uint8_t key)
{
	switch (key) {

		case	MKI_KEY_1:
		case	MKI_KEY_2:
		case	MKI_KEY_3:
		case	MKI_KEY_4:
		case	MKI_KEY_5:
		case	MKI_KEY_6:
		case	MKI_KEY_7:
		case	MKI_KEY_8:
		case	MKI_KEY_9:
			strcat_s(setevoe_upravlenie_addr_pu, sizeof(setevoe_upravlenie_addr_pu), int2str(key));
			screen4_paint();
		break;
		case	MKI_KEY_10:
			strcat_s(setevoe_upravlenie_addr_pu, sizeof(setevoe_upravlenie_addr_pu), "0");
			screen4_paint();
		break;
		case MKI_KEY_DELETE:
			if (strlen(setevoe_upravlenie_addr_pu)) {
				setevoe_upravlenie_addr_pu[strlen(setevoe_upravlenie_addr_pu) - 1] = '\0';
				screen4_paint();
			}
		break;
		case MKI_KEY_RESET:
		case MKI_KEY_MENU:
		case MKI_KEY_ENTER:
			screens_goto(0);
		break;
	}
}

void screen4_button_unpress(uint8_t key)
{

}
