#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>
#include <ntddscsi.h>		
#include "N6_platdef.h"
#include "Debug.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")//���þ���

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
#ifdef ALLOC_PRAGMA//���ϵͳ�����������Ҳ����֧���ַ���
#pragma alloc_text(INIT, DriverEntry)//����ں�������������������ɳ�ʼ��������������Ҫ������ֱ�Ӱ������뵽INIT�Ρ�
#pragma alloc_text(PAGE, NPUnload)//�����ҳ�ڴ���
#pragma alloc_text(PAGE, NPInstanceQueryTeardown)
#pragma alloc_text(PAGE, NPInstanceSetup)
#pragma alloc_text(PAGE, NPInstanceTeardownStart)
#pragma alloc_text(PAGE, NPInstanceTeardownComplete)
#pragma alloc_text(PAGE, NPPreCreate)//�����������ҳ�ڴ棬��ʡ�Ƿ�ҳ�ڴ���Դ

#pragma alloc_text(PAGE, NPMiniConnect)				//for port comunication
#pragma alloc_text(PAGE, NPMiniDisconnect)			//for port comunication
#pragma alloc_text(PAGE, NPMiniMessage)    		//for port comunication		
#endif    

//  operation registration
const FLT_OPERATION_REGISTRATION Callbacks[] = {
	{ IRP_MJ_READ,//��Ҫ���˵�irp����
	0,//��2������һ����־λ��������д��:��1����д0,�����־�����Զ�/д�ص����ã����Զ���������Ĵ���ֱ��д0����:
	//��2����дFLTFL OPERATION_ REGISTRATION_SKIP_CACHED_I0,��ʾ�����˻����/д����
	//��3����дFLTFL OPERATION REGISTRATION_SKIP_ PAGING_IO, ��ʾ�����˷�ҳ��/д����
	NULL,//irp���·������л���õĺ���
	MyPostRead//irp��ɺ󷵻�ʱ���õĺ���
	//����������������˵�ʱ�����ΪNULL
	},




	{ IRP_MJ_CREATE,
	0,
	NPPreCreate,
	NULL },

	{ IRP_MJ_OPERATION_END }
};

//  This defines what we want to filter with FltMgr
const FLT_REGISTRATION FilterRegistration = {

	sizeof(FLT_REGISTRATION),         //  ��С
	FLT_REGISTRATION_VERSION,           //  �汾
	0,                                  //  Flags

	NULL,                               //  Context
	Callbacks,                          //  ��Ҫ���˵�irp�Ĵ�����
	NPUnload,                           //  ж�غ���

	NPInstanceSetup,                    //  ���ھ�����Щ����Ҫ��
	NPInstanceQueryTeardown,            //  ����ʵ�����ٺ���
	NPInstanceTeardownStart,            //  ����󶨻ص�����
	NPInstanceTeardownComplete,         //  �������ɺ���

	NULL,                               //  ���ɵ��ļ���
	NULL,                               //  ����Ŀ���ļ���
	NULL                                //  ��׼���������

};





