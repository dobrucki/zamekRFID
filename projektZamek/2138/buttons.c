#include "buttons.h"

tU8 checkButton(tU32 key) {

	tU8 wcisnietyKlawisz;

	if ((IOPIN1 & key) == 0) {
		osSleep(5);
	if ((IOPIN1 & key) == 0) {
		wcisnietyKlawisz = key;
		return TRUE;
	} else {
		wcisnietyKlawisz = 0;
		return FALSE;
	}
	} else {
		wcisnietyKlawisz = 0;
		return FALSE;
	}
}
