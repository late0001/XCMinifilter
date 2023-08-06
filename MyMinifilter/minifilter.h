#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntddscsi.h>		
#include "N6_platdef.h"
#include "Debug.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")//禁用警告

#define PTDBG_TRACE_ROUTINES            0x00000001
#define PTDBG_TRACE_OPERATION_STATUS    0x00000002

#define MINISPY_PORT_NAME								L"\\MYMiniPort"
int command = 1;
PUCHAR str1;
PUCHAR str2;
ULONG gTraceFlags = 0;
HANDLE myhandl;
HANDLE myhand2;
PFLT_FILTER gFilterHandle;
PFLT_PORT 	gServerPort;
PFLT_PORT 	gClientPort;
int ONOROFF = 0;
int FirsT = 0;
int ddd = 0;
int FalgPassWord = 1;
BOOLEAN NPUnicodeStringToChar(PUNICODE_STRING UniName, char Name[]);



NTSTATUS
DriverEntry(
__in PDRIVER_OBJECT DriverObject,
__in PUNICODE_STRING RegistryPath
);

NTSTATUS
NPInstanceSetup(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in FLT_INSTANCE_SETUP_FLAGS Flags,
__in DEVICE_TYPE VolumeDeviceType,
__in FLT_FILESYSTEM_TYPE VolumeFilesystemType
);

VOID
NPInstanceTeardownStart(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in FLT_INSTANCE_TEARDOWN_FLAGS Flags
);

VOID
NPInstanceTeardownComplete(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in FLT_INSTANCE_TEARDOWN_FLAGS Flags
);

NTSTATUS
NPUnload(
__in FLT_FILTER_UNLOAD_FLAGS Flags
);

NTSTATUS
NPInstanceQueryTeardown(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
);

FLT_PREOP_CALLBACK_STATUS
NPPreCreate(
__inout PFLT_CALLBACK_DATA Data,
__in PCFLT_RELATED_OBJECTS FltObjects,
__deref_out_opt PVOID *CompletionContext
);



NTSTATUS
NPMiniMessage(
__in PVOID ConnectionCookie,
__in_bcount_opt(InputBufferSize) PVOID InputBuffer,
__in ULONG InputBufferSize,
__out_bcount_part_opt(OutputBufferSize, *ReturnOutputBufferLength) PVOID OutputBuffer,
__in ULONG OutputBufferSize,
__out PULONG ReturnOutputBufferLength
);

NTSTATUS
NPMiniConnect(
__in PFLT_PORT ClientPort,
__in PVOID ServerPortCookie,
__in_bcount(SizeOfContext) PVOID ConnectionContext,
__in ULONG SizeOfContext,
__deref_out_opt PVOID *ConnectionCookie
);


VOID
NPMiniDisconnect(
__in_opt PVOID ConnectionCookie
);

FLT_POSTOP_CALLBACK_STATUS FLTAPI
MyPostRead(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags
);

BOOLEAN NPUnicodeStringUpper(PUNICODE_STRING UniName, wchar_t Name[]);

//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA//如果系统定义了这个，也就是支持种方法
#pragma alloc_text(INIT, DriverEntry)//像入口函数代码在驱动程序完成初始化后往往不再需要，可以直接把它插入到INIT段。
#pragma alloc_text(PAGE, NPUnload)//放入分页内存中
#pragma alloc_text(PAGE, NPInstanceQueryTeardown)
#pragma alloc_text(PAGE, NPInstanceSetup)
#pragma alloc_text(PAGE, NPInstanceTeardownStart)
#pragma alloc_text(PAGE, NPInstanceTeardownComplete)
#pragma alloc_text(PAGE, NPPreCreate)//将函数放入分页内存，节省非分页内存资源

#pragma alloc_text(PAGE, NPMiniConnect)				//for port comunication
#pragma alloc_text(PAGE, NPMiniDisconnect)			//for port comunication
#pragma alloc_text(PAGE, NPMiniMessage)    		//for port comunication		
#endif    

//  operation registration
const FLT_OPERATION_REGISTRATION Callbacks[] = {
	{ IRP_MJ_READ,//需要过滤的irp种类
	0,//第2个域是一个标志位，有三种写法:第1种是写0,这个标志仅仅对读/写回调有用，所以对生成请求的处理直接写0即可:
	//第2种是写FLTFL OPERATION_ REGISTRATION_SKIP_CACHED_I0,表示不过滤缓冲读/写请求
	//第3种是写FLTFL OPERATION REGISTRATION_SKIP_ PAGING_IO, 表示不过滤分页读/写请求
	NULL,//irp向下发过程中会调用的函数
	MyPostRead//irp完成后返回时调用的函数
	//这两个函数不想过滤的时候可以为NULL
	},




	{ IRP_MJ_CREATE,
	0,
	NPPreCreate,
	NULL },

	{ IRP_MJ_OPERATION_END }
};

//  This defines what we want to filter with FltMgr
const FLT_REGISTRATION FilterRegistration = {

	sizeof(FLT_REGISTRATION),         //  大小
	FLT_REGISTRATION_VERSION,           //  版本
	0,                                  //  Flags

	NULL,                               //  Context
	Callbacks,                          //  需要过滤的irp的处理函数
	NPUnload,                           //  卸载函数

	NPInstanceSetup,                    //  用于决定那些卷需要绑定
	NPInstanceQueryTeardown,            //  控制实例销毁函数
	NPInstanceTeardownStart,            //  解除绑定回调函数
	NPInstanceTeardownComplete,         //  解除绑定完成函数

	NULL,                               //  生成的文件名
	NULL,                               //  生成目标文件名
	NULL                                //  标准化组件名称

};





