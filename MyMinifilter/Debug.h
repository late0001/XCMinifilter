#ifndef __INC_DEBUG_H
#define __INC_DEBUG_H
#include "N6_platdef.h"

#define BIT0		0x00000001
#define BIT1		0x00000002
#define BIT2		0x00000004
#define BIT3		0x00000008
#define BIT4		0x00000010
#define BIT5		0x00000020
#define BIT6		0x00000040
#define BIT7		0x00000080
#define BIT8		0x00000100
#define BIT9		0x00000200
#define BIT10	0x00000400
#define BIT11	0x00000800
#define BIT12	0x00001000
#define BIT13	0x00002000
#define BIT14	0x00004000
#define BIT15	0x00008000
#define BIT16	0x00010000
#define BIT17	0x00020000
#define BIT18	0x00040000
#define BIT19	0x00080000
#define BIT20	0x00100000
#define BIT21	0x00200000
#define BIT22	0x00400000
#define BIT23	0x00800000
#define BIT24	0x01000000
#define BIT25	0x02000000
#define BIT26	0x04000000
#define BIT27	0x08000000
#define BIT28	0x10000000
#define BIT29	0x20000000
#define BIT30	0x40000000
#define BIT31	0x80000000

#define DBG_OFF					0

//
//	Deprecated! Don't use it! 
//	TODO: fix related debug message!
//
//#define DBG_SEC					1

//
//	Fatal bug. 
//	For example, Tx/Rx/IO locked up, OS hangs, memory access violation, 
//	resource allocation failed, unexpected HW behavior, HW BUG and so on.
//
#define DBG_SERIOUS				2	// WPP Error condition

//
//	Abnormal, rare, or unexpeted cases.
//	For example, IRP/Packet/OID canceled, device suprisely unremoved and so on.
//
#define DBG_WARNING			3	// WPP Warning

//
//	Normal case with useful information about current SW or HW state. 
//	For example, Tx/Rx descriptor to fill, Tx/Rx descriptor completed status, 
//	SW protocol state change, dynamic mechanism state change and so on.
//
#define DBG_LOUD				4	// WPP Information

//
//	Normal case with detail execution flow or information.
//
#define DBG_TRACE				5	//WPP most detailed trace

#define RTW_U32_MAX 0xFFFFFFFF
#define COMP_DBG				BIT0		// For function call tracing.
#define COMP_INIT			BIT1	// during driver initialization / halt / reset.

extern u4Byte GlobalDebugLevel;
extern u8Byte GlobalDebugComponents;

#define RT_TRACE(_Comp, _Level, Fmt)												\
			if(((_Comp) & GlobalDebugComponents) && ((_Level <= GlobalDebugLevel) || (_Level == DBG_SERIOUS)))	\
			{																	\
				if(DBG_SERIOUS == _Level) DbgPrint("[ERROR]");	\
				else if(DBG_WARNING == _Level) DbgPrint("[WARNING]");	\
				DbgPrint Fmt;														\
			}

#endif