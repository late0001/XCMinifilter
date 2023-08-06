#include <wdm.h>
#include "N6_platdef.h"
#include "Debug.h"

u8Byte	RTL_AllocateMemory_count = 0;
u8Byte	RTL_AllocateMemory_Len = 0;
u8Byte	RTL_FreeMemory_count = 0;
u8Byte	RTL_FreeMemory_Len = 0;
//NPAGED_LOOKASIDE_LIST Pre2PostContextList;

u4Byte
GenTag(
	IN	char* pFunName
)
{
	u2Byte	units, tens = 0;
	u4Byte	tag;

	RT_TRACE(COMP_INIT, DBG_TRACE, ("GenTag() by %s()", pFunName));
	units = (u2Byte)strlen(pFunName);
	tag = pFunName[0] | (pFunName[units - 1] << 8);

	while (units >= 10)
	{
		units -= 10;
		tens++;
	}
	tag |= ((tens + 0x30) << 16) | ((units + 0x30) << 24);

	return tag;
}

#define _os_mem_alloc(h, buf_sz) _os_mem_alloc_with_tag(h, GenTag(__func__), buf_sz)
/* memory */
static __inline void* _os_mem_alloc_with_tag(void* h, u4Byte tag, u4Byte buf_sz)
{
	PVOID ptr = NULL;
	if (PlatformAllocateMemoryWithTag(tag, &ptr, buf_sz) != KC_STATUS_SUCCESS)
		return NULL;
	//PPRE_2_POST_CONTEXT p2pCtx = ExAllocateFromNPagedLookasideList(&Pre2PostContextList);
	PlatformZeroMemory(ptr, buf_sz);
	return ptr;
}

static __inline void _os_mem_free(void* h, void* buf, u4Byte buf_sz)
{
	if (buf)
		PlatformFreeMemory(buf, buf_sz);
}

KC_STATUS
PlatformAllocateMemoryWithTag(
	IN  u4Byte      Tag,
	OUT	PVOID* pPtr,
	IN	u4Byte		length
)
{
	KC_STATUS		rtstatus = KC_STATUS_SUCCESS;

	RT_TRACE(COMP_INIT, DBG_TRACE, (", Tag=%c%c%c%c, allocate size = 0x%x\n",
		(u1Byte)(Tag & 0xff), (u1Byte)((Tag & 0xff00) >> 8), (u1Byte)((Tag & 0xff0000) >> 16), (u1Byte)((Tag & 0xff000000) >> 24), length));

	//*pPtr = ExAllocatePoolWithTagPriority(NonPagedPool,
	//	length,
	//	Tag,
	//	NormalPoolPriority);

	POOL_EXTENDED_PARAMETER ExtendedParameters = { PoolExtendedParameterPriority , NormalPoolPriority };
	*pPtr = ExAllocatePool3(NonPagedPool, length, Tag, &ExtendedParameters, 1);
	if (*pPtr == NULL)
	{
		return KC_STATUS_FAILURE;
	}

	RTL_AllocateMemory_count++;
	RTL_AllocateMemory_Len += length;

	return rtstatus;
}

//
// Description:
//	Free the allocated memory of the input pointer.
//
VOID
PlatformFreeMemory(
	IN	PVOID		ptr,
	IN	u4Byte		length
)
{
	if (ptr)
	{
		ExFreePool(ptr);
		ptr = NULL;
		RTL_FreeMemory_count++;
		RTL_FreeMemory_Len += length;

		//		RTL_AllocateMemory_Len -= length;
	}
}

//
// Description:
//	Fill all the input memory with 0.
//
VOID
PlatformZeroMemory(
	IN	PVOID		ptr,
	IN	u4Byte		length
)
{
	if (ptr)
		RtlZeroMemory(ptr, length);
}