#include "Debug.h"
#include "N6_platdef.h"

u4Byte GlobalDebugLevel = DBG_LOUD;
#if DBG
u8Byte GlobalDebugComponents = \
//									COMP_DBG				|
COMP_INIT |
	0;
#endif 