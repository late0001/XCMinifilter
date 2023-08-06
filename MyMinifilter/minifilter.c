#include "minifilter.h"

void DriverUnload(PDRIVER_OBJECT pDriver)
{
	RT_TRACE(COMP_INIT, DBG_LOUD, ("DriverUnload ==>"));
	if (pDriver->DeviceObject)
	{
		IoDeleteDevice(pDriver->DeviceObject);
		//UNICODE_STRING sybolName = { 0 };
		//RtlInitUnicodeString(&sybolName, SYM_NAME);
		//IoDeleteSymbolicLink(&sybolName);
	}

}

NTSTATUS
DriverEntry(
__in PDRIVER_OBJECT DriverObject,  //PDRIVER_OBJECT �������ݽṹ���������󣩵�ָ��
__in PUNICODE_STRING RegistryPath//PUNICODE_STRING �ں��ַ������飬�����Է������ʽ���أ�����ַ���Ϊ������ע����·��
)
{
	NTSTATUS status;              //����ֵ״̬
	PSECURITY_DESCRIPTOR sd;     //��ȫ��������
	OBJECT_ATTRIBUTES oa;        //��������
	UNICODE_STRING uniString;		//����ͨ�Ŷ˿�����

	UNREFERENCED_PARAMETER(RegistryPath);//�������������δ���ò����ľ���
	DriverObject->DriverUnload = DriverUnload;
	status = STATUS_SUCCESS;
	RT_TRACE(COMP_INIT, DBG_LOUD, ("DriverEntry ==>\n"));
	//return status;
#if 1
	// ע��FilterRegistration������ϵͳ�����õĻص�����
	//��1�������Ǳ���������������������ں���DriverEntry ����Ϊ��������ġ�
	//��2����������һ��ע����Ϣ�Ľṹ������ṹ�ں����������������������Ϣ
	//��3������ ��һ�����ز���������ע��ɹ���΢�������������������ú���FltStartFilteringʱ���õ�
	//DbgPrint("statuswww=\n");
	status = FltRegisterFilter(DriverObject,
		&FilterRegistration,
		&gFilterHandle);

	ASSERT(NT_SUCCESS(status));//������Ƿ�ɹ���ʧ�ܺ󴥷��쳣���������ӹ�

	if (NT_SUCCESS(status)) {//������Ƿ�ɹ���

		//
		//  ��ʼ������ֻ��һ������Ϊ֮ǰ��ȡ�ľ��
		//

		status = FltStartFiltering(gFilterHandle);

		if (!NT_SUCCESS(status)) {
			//���ʧ�ܾ�ȡ��ע�ᣬֻ��һ������Ϊ֮ǰ��ȡ�ľ��
			FltUnregisterFilter(gFilterHandle);
		}
	}

	//FltBuildDefaultSecurityDescriptor����һ��Ĭ�ϵİ�ȫ������������FltCreateCommunicationPort��
	//����1ָ����÷�����ı�����ָ�룬�ñ������յ�ָ���´����Ĳ�͸��ָ��
	//����2ָ����������Ҫ�Զ˿ڶ���ķ������͵ı�־��λ���롣ϵͳ�����DesiredAccess��־��ȷ����minifilter��������ͨ�Ŷ˿ڶ���������ض�����Ȩ�ޡ�
	status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);

	if (!NT_SUCCESS(status)) {//������Ƿ�ɹ���δ�ɹ����������մ�����
		goto final;
	}

	//������ʼ��Unicode�ַ��ļ����ַ���
	RtlInitUnicodeString(&uniString, MINISPY_PORT_NAME);

	//���ʼ����͸����OBJECT_ATTRIBUTES�ṹ���ýṹ��������������ָ�����򿪾�������̡�
	//����1ΪҪ����ʼ���Ľṹ�壬����2һ��ָ��Unicode�ַ�����ָ�룬���ַ�������ҪΪ��򿪾���Ķ�������ơ�
	//�������һ����ȫ�޶��Ķ�������������RootDirectory����ָ���Ķ���Ŀ¼�����·������
	//����3��־λ���˴�ָ��Ϊֻ���ں�ģʽ���ʣ���Դ�Сд������
	//����4��ObjectName������ָ����·�����ĸ�����Ŀ¼��������ObjectName����ȫ�޶��Ķ������ƣ�
	//��RootDirectoryΪ�ա�ʹ��ZwCreateDirectoryObject��ö���Ŀ¼�ľ����
	//����5ָ����������ʱӦ���ڸö���İ�ȫ���������˲����ǿ�ѡ�ġ������������ָ��NULL�����ܶ����Ĭ�ϰ�ȫ�ԡ�
	InitializeObjectAttributes(&oa,
		&uniString,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL,
		sd);

	//����һ��ͨ�ŷ������˿ڣ�minifilter������������ڸö˿��Ͻ��������û�ģʽӦ�ó������������
	// ����1�����ߵĲ�͸��������ָ��
	//����2ָ����÷�����ı�����ָ�룬�ñ�������ͨ�ŷ������˿ڵĲ�͸���˿ھ����minifilter��������ʹ�������������������û�ģʽӦ�ó������������
	//����3ָ��OBJECT_ATTRIBUTES�ṹ��ָ�룬�ýṹָ��ͨ�ŷ������˿ڵ����ԡ�
	//�˽ṹ������֮ǰ��InitializeObjectAttributes���ó�ʼ�����˲����Ǳ���ģ�����Ϊ�ա�ͨ�Ŷ˿ڶ���Ĵ˽ṹ�ĳ�Ա�����������ݡ�
	//����4ָ����minifilter�������������������Ϣ��ָ�롣�����Ϣ����������������ͬ��minifilter�������򴴽��Ķ��ͨ�ŷ������˿ڡ�
	//�����������������������ָ����Ϊ�������ݸ�ConnectNotifyCallback���̡��ò����ǿ�ѡ�ģ�����ΪNULL��
	//����5 NPMiniConneet���û�̬���ں�̬��������ʱ�ں˻���õ��ĺ�����
	//����6 NPMiniDisconnect ���û�̬���ں�̬���ӽ���ʱ�ں˻���õ��ĺ�������
	//����7 NPMiniMessage���û�̬���ں�̬��������ʱ�ں˻���õ��ĺ���
	//����8�˷������˿����������󲢷��ͻ������������˲����Ǳ���ģ��ұ�������㡣
	status = FltCreateCommunicationPort(gFilterHandle,
		&gServerPort,
		&oa,
		NULL,
		NPMiniConnect,
		NPMiniDisconnect,
		NPMiniMessage,
		1);
	//�ͷŷ���İ�ȫ������
	FltFreeSecurityDescriptor(sd);

	if (!NT_SUCCESS(status)) {
		goto final;
	}

	// ����ֵ

	// ���ȳ�ʼ�������ļ�·����OBJECT_ATTRIBU
	UNICODE_STRING mytext1;  //�ļ����ַ���
	OBJECT_ATTRIBUTES ATTRIBUTES1;
	IO_STATUS_BLOCK BLOCK1;//��ӵ�еĿ���Ȩ  
	LARGE_INTEGER size1;  //д��ƫ�ƴ�С

	str1 = ExAllocatePool(NonPagedPool, 500);//����500�ķǷ�ҳ�ڴ�
	RtlZeroMemory(str1, 500);//��0�ڴ�

	RtlInitUnicodeString(&mytext1, L"\\??\\C:\\true.txt");//��ʼ���ļ���  
	memset(&ATTRIBUTES1, 0, sizeof(OBJECT_ATTRIBUTES));                        //�����������  
	size1.QuadPart = 0;

	InitializeObjectAttributes(&ATTRIBUTES1, &mytext1, OBJ_CASE_INSENSITIVE, NULL, NULL);//�������Թؼ����ļ����� �����ִ�Сд ����������ͬ��

	//��Ŀ���ļ���ȡ���
	//����1��һ��ָ����������ָ�룬�þ�����ڽ����ļ��ľ����
	//����2��ָ��ACCESS_MASKֵ����ֵȷ������Զ���ķ��ʡ�����Ϊ�������͵Ķ�����ķ���Ȩ��֮�⣬�����߻�����ָ�������ض����ļ��ķ���Ȩ�ޡ�
	//����3��OBJECT_ATTRIBUTES�ṹ��ָ�룬�ýṹָ�����������������ԡ�ʹ��InitializeObjectAttributes��ʼ���ýṹ��
	//����4�����ڽ����������״̬�����������������Ϣ
	//����5�������������򸲸ǵ��ļ��ĳ�ʼ�����С������ΪNull
	//����6����ʾ�����򸲸��ļ�ʱҪ���õ��ļ����ԡ�������ͨ��ָ��FILE_ATTRIBUTE_NORMAL��������Ĭ������
	//����7��������ʵ����ͣ�
	//����8��ָ�����ļ����ڻ򲻴���ʱִ�еĲ���
	//����9��ָ���������򴴽�����ļ�ʱӦ�õ�ѡ��
	//����10�������豸���м��������򣬴˲�������Ϊ��ָ��
	//����11�������豸���м��������򣬴˲�������Ϊ�㡣
	status = ZwCreateFile(&myhandl, GENERIC_ALL, &ATTRIBUTES1, &BLOCK1, NULL, FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ, FILE_OPEN_IF, FILE_NON_DIRECTORY_FILE, NULL, 0);

	//ͨ�������ȡĿ���ļ�����str1
	if (!NT_SUCCESS(status))
	{
		FirsT = 0;
		goto final;
	}
	else
	{
		status = ZwReadFile(myhandl, NULL, NULL, NULL, &BLOCK1, str1, 500, &size1, NULL);
		FirsT = 1;//��ֹ����
		DbgPrint("status=%X\n", status);

	}

	//�������ݻ���ͬ��ֻ���ļ��Ķ���һ��
	UNICODE_STRING mytext2;
	OBJECT_ATTRIBUTES ATTRIBUTES2;
	IO_STATUS_BLOCK BLOCK2;//��ӵ�еĿ���Ȩ  
	LARGE_INTEGER size2;
	str2 = ExAllocatePool(NonPagedPool, 500);
	RtlZeroMemory(str2, 500);
	RtlInitUnicodeString(&mytext2, L"\\??\\C:\\false.txt");//��ʼ���ļ���  
	memset(&ATTRIBUTES2, 0, sizeof(OBJECT_ATTRIBUTES));                        //�����������  
	size2.QuadPart = 0;
	InitializeObjectAttributes(&ATTRIBUTES2, &mytext2, OBJ_CASE_INSENSITIVE, NULL, NULL);//�������Թؼ����ļ����� �����ִ�Сд  
	status = ZwCreateFile(&myhand2, GENERIC_ALL, &ATTRIBUTES2, &BLOCK2, NULL, FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ, FILE_OPEN_IF, FILE_NON_DIRECTORY_FILE, NULL, 0);
	if (!NT_SUCCESS(status))
	{
		FirsT = 0;
		goto final;
	}
	else
	{
		status = ZwReadFile(myhand2, NULL, NULL, NULL, &BLOCK2, str2, 500, &size2, NULL);
		DbgPrint("status2=%X\n", status);

	}



	//��Ŀ���ļ�true.txt����д�룬�Ӷ�����read
	PVOID str3 = ExAllocatePool(PagedPool, 10);
	RtlZeroMemory(str3, 6);
	RtlCopyMemory(str3, "...", strlen("..."));
	size1.HighPart = 0;
	size1.QuadPart = 500;
	status = ZwWriteFile(myhandl, NULL, NULL, NULL, &BLOCK1, str3, 6, &size1, NULL);


	//�رվ��
	ZwClose(myhandl);
	ZwClose(myhand2);
#endif
	final ://ʧ�ܴ���

	if (!NT_SUCCESS(status)) {

		if (NULL != gServerPort) {
			//�رչ��������������ͨ�ŷ������˿�
			FltCloseCommunicationPort(gServerPort);
		}

		if (NULL != gFilterHandle) {
			//ȡ��ע��
			FltUnregisterFilter(gFilterHandle);
		}
	}
	return status;
}

ULONG FltFilterOperationsOffset = 0x1a8; //WIN11 OFFSET of fltmgr!_FLT_FILTER->PFLT_OPERATION_REGISTRATION
ULONG EnumMiniFilter()
{
	long  ntStatus;
	ULONG  uNumber;
	PVOID  pBuffer = NULL;
	ULONG  uIndex = 0, DrvCount = 0;
	PVOID  pCallBacks = NULL, pFilter = NULL;
	PFLT_OPERATION_REGISTRATION pNode;
	do
	{
		if (pBuffer != NULL)
		{
			ExFreePool(pBuffer);
			pBuffer = NULL;
		}
		ntStatus = FltEnumerateFilters(NULL, 0, &uNumber);
		if (ntStatus != STATUS_BUFFER_TOO_SMALL)
			break;
		pBuffer = ExAllocatePoolWithTag(NonPagedPool, sizeof(PFLT_FILTER) * uNumber, 'mnft');
		if (pBuffer == NULL)
		{
			RT_TRACE(COMP_INIT, DBG_SERIOUS, (" %s(%d)ExAllocatePoolWithTag failed!\n",__FUNCTION__ ,__LINE__));
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			break;
		}
		ntStatus = FltEnumerateFilters(pBuffer, uNumber, &uNumber);
	} while (ntStatus == STATUS_BUFFER_TOO_SMALL);
	if (!NT_SUCCESS(ntStatus))
	{
		if (pBuffer != NULL)
			ExFreePool(pBuffer);
		return 0;
	}
	RT_TRACE(COMP_INIT, DBG_LOUD,  ("MiniFilter Count: %ld\n", uNumber));
	RT_TRACE(COMP_INIT, DBG_LOUD, ("------\n"));
	__try
	{
		while (DrvCount < uNumber)
		{
			pFilter = (PVOID)(*(PULONG64)((PUCHAR)pBuffer + DrvCount * 8));
			pCallBacks = (PVOID)((PUCHAR)pFilter + FltFilterOperationsOffset);
			pNode = (PFLT_OPERATION_REGISTRATION)(*(PULONG64)pCallBacks);
			__try
			{
				while (pNode->MajorFunction != 0x80)  //IRP_MJ_OPERATION_END
				{
					if (pNode->MajorFunction < 28)  //MajorFunction id is 0~27
					{
						RT_TRACE(COMP_INIT, DBG_LOUD, ("Object=%p\tPreFunc=%p\tPostFunc=%p\tIRP=%d\n",
							pFilter,
							pNode->PreOperation,
							pNode->PostOperation,
							pNode->MajorFunction));
					}
					pNode++;
				}
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				FltObjectDereference(pFilter);
				RT_TRACE(COMP_INIT, DBG_LOUD, ("[EnumMiniFilter] EXCEPTION_EXECUTE_HANDLER: pNode->MajorFunction\n"));
				ntStatus = GetExceptionCode();
				ExFreePool(pBuffer);
				return uIndex;
			}
			DrvCount++;
			FltObjectDereference(pFilter);
			RT_TRACE(COMP_INIT, DBG_LOUD, ("------\n"));
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		FltObjectDereference(pFilter);
		RT_TRACE(COMP_INIT, DBG_LOUD, ("[EnumMiniFilter] EXCEPTION_EXECUTE_HANDLER\n"));
		ntStatus = GetExceptionCode();
		ExFreePool(pBuffer);
		return uIndex;
	}
	if (pBuffer != NULL)
	{
		ExFreePool(pBuffer);
		ntStatus = STATUS_SUCCESS;
	}
	return uIndex;
}



FLT_POSTOP_CALLBACK_STATUS 
MyPostRead(
_Inout_ PFLT_CALLBACK_DATA Data,
_In_ PCFLT_RELATED_OBJECTS FltObjects,
_In_opt_ PVOID CompletionContext,
_In_ FLT_POST_OPERATION_FLAGS Flags
)
{

	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(CompletionContext);
	char FileName[260] = "X:";//��Ž�������ļ�����
	wchar_t wFileName[260] = L"X:";//��Ž�������ļ�����
	NTSTATUS status;  //����ֵ״̬
	PFLT_FILE_NAME_INFORMATION nameInfo; //������Ϣ
	RT_TRACE(COMP_INIT, DBG_LOUD, ("MyPostRead ==>\n"));
	//
	// ��ʧ�ܵ�irp���й��ˣ����ʧ���ˣ��Ͳ��ü���
	if (!NT_SUCCESS(Data->IoStatus.Status) ||
		(STATUS_REPARSE == Data->IoStatus.Status)) {

		return FLT_POSTOP_FINISHED_PROCESSING;
	}



	//FltGetFileNameInformation���̷����ļ���Ŀ¼��������Ϣ��               								
	status = FltGetFileNameInformation(Data,//PFLT_CALLBACK_DATA�ṹ��ָ�룬����I/O�����Ļص����ݽṹ���˲����Ǳ���ģ�����Ϊ�ա�
		FLT_FILE_NAME_NORMALIZED |//ʹnameInfo�������հ����ļ��淶�����ƵĽṹ�ĵ�ַ��
		FLT_FILE_NAME_QUERY_DEFAULT,//�����ѯ�ļ�ϵͳ�Ի�ȡ�ļ���Ŀǰ������ȫ����FltGetFileNameInformation����ִ���κβ�����
		//����FltGetFileNameInformation��ѯ�����������������ƻ����Ի�ȡ�ļ�����Ϣ��
		//����ڻ�����û���ҵ������ƣ���FltGetFileNameInformation��ѯ�ļ�ϵͳ����������
		&nameInfo);//�����ļ�����Ϣ��

	if (NT_SUCCESS(status))
	{
		//����FLT_FILE_NAME_INFORMATION�ṹ�����ݡ�
		status = FltParseFileNameInformation(nameInfo);


		//����1��Сдת������������2
		//if (NPUnicodeStringToChar(&nameInfo->Name, FileName))
		RT_TRACE(COMP_INIT, DBG_LOUD, ("MyPostRead() &nameInfo->Name= %s\n", nameInfo->Name.Buffer));
		if(NPUnicodeStringUpper(&nameInfo->Name, wFileName))
		{
			
			//DbgPrint("FileName is %s\n", FileName);
			//if (strstr(FileName, "TRUE.TXT") > 0)
			if(wcsstr(wFileName,L"TRUE.TXT") > 0)
			{
				PUCHAR buf1;
				RT_TRACE(COMP_INIT, DBG_LOUD, ("MyPostRead() True.txt, Command= %d, FirsT = %d\n", command, FirsT));
				if (Data->Iopb->Parameters.Read.MdlAddress != 0)
				{
					//���Ϊmdl��ʽ��ȡ�ļ����ͻ�ȡ��ַ
					buf1 = MmGetSystemAddressForMdlSafe(Data->Iopb->Parameters.Read.MdlAddress, NormalPagePriority);

				}
				else
				{
					//���ΪReadBuffer��ֱ�Ӹ�ָ��
					buf1 = Data->Iopb->Parameters.Read.ReadBuffer;
				}
				//�ж�Ӧ�ò㷢����ָ���Ƿ�Ϊ��ʾtrue.txt
				if (command == 2)
				{
					if (FirsT == 1)
					{
						strcpy(str1, "1111111111111");
						//�����ڴ棬��д����
						RtlCopyMemory(buf1, str1, 500);
						Data->IoStatus.Information = Data->Iopb->Parameters.Read.Length;
					}
				}
				else
				{
					//DbgBreakPoint();
					if (FirsT == 1)
					{
						strcpy(str2, "000000000000");
						//�����ڴ棬��д����
						RtlCopyMemory(buf1, str2, 500);
						Data->IoStatus.Information = Data->Iopb->Parameters.Read.Length;
					}

				}
				DbgPrint("ReadBuffed=%s", buf1);
				//�ͷ��ļ�����Ϣ�ṹ����ڴ�
				FltReleaseFileNameInformation(nameInfo);
				return FLT_POSTOP_FINISHED_PROCESSING;
			}
		}

		FltReleaseFileNameInformation(nameInfo);
	}

	return FLT_POSTOP_FINISHED_PROCESSING;

}

NTSTATUS
NPMiniMessage(
__in PVOID ConnectionCookie,
__in_bcount_opt(InputBufferSize) PVOID InputBuffer,
__in ULONG InputBufferSize,
__out_bcount_part_opt(OutputBufferSize, *ReturnOutputBufferLength) PVOID OutputBuffer,
__in ULONG OutputBufferSize,
__out PULONG ReturnOutputBufferLength
)
{
	//д���ļ���Ҫ�ĸ��ֲ���������ͬdirverEntry
	NTSTATUS status;
	UNICODE_STRING mytext1;
	OBJECT_ATTRIBUTES ATTRIBUTES1;
	IO_STATUS_BLOCK BLOCK1;//��ӵ�еĿ���Ȩ  
	LARGE_INTEGER size1;
	PVOID str4 = ExAllocatePool(PagedPool, 40);

	RT_TRACE(COMP_INIT, DBG_LOUD, ("NPMiniMessage ==>\n"));
	RtlZeroMemory(str4, 40);
	RtlInitUnicodeString(&mytext1, L"\\??\\C:\\true.txt");//��ʼ���ļ���  
	memset(&ATTRIBUTES1, 0, sizeof(OBJECT_ATTRIBUTES));                        //�����������  
	size1.QuadPart = 0;


	PAGED_CODE();
	//�����ڴ棬�洢�ַ��������ں�Ӧ�ò��ַ����Ƚ�
	PVOID test;
	PVOID test2;
	PVOID test3;
	test = ExAllocatePool(PagedPool, 10);
	RtlCopyMemory(test, L"0", wcslen(L"0"));
	
	test2 = ExAllocatePool(PagedPool, 50);
	RtlCopyMemory(test2, L"52pojie", wcslen(L"52pojie"));
	
	UNREFERENCED_PARAMETER(ConnectionCookie);
	UNREFERENCED_PARAMETER(OutputBufferSize);
	UNREFERENCED_PARAMETER(OutputBuffer);
	RT_TRACE(COMP_INIT, DBG_LOUD, ("NPMiniMessage() Input:%ws \n", InputBuffer));
	//ͨ��Ӧ�ò�������ַ���������ʾ��Ӧ���ı�
	if (wcscmp(InputBuffer, test) == 0)
	{
		command = 1;
	}
	else if (wcscmp(InputBuffer, test2) == 0)
	{
		command = 2;
	}
	else if (wcsncmp(InputBuffer, L"EnumFilter", wcslen(L"EnumFilter")) == 0)
	{
		command = 3;
	}
	else
	{
		status = STATUS_INVALID_PARAMETER;

	}
	RT_TRACE(COMP_INIT, DBG_LOUD, ("NPMiniMessage() command:%d \n", command));
	
	if (command == 3)
		EnumMiniFilter();

	//���ļ�д�룬������read��������Ҫ���ݲ�ͬ������ͨ���ı�ONOROFF��ʵ������д������ִ��
	if (ONOROFF == 0)
	{
		//���ļ�д��ո�
		ONOROFF = ONOROFF + 1;
		InitializeObjectAttributes(&ATTRIBUTES1, &mytext1, OBJ_CASE_INSENSITIVE, NULL, NULL);//�������Թؼ����ļ����� �����ִ�Сд  
		status = ZwCreateFile(&myhandl, GENERIC_ALL, &ATTRIBUTES1, &BLOCK1, NULL, FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ, FILE_OPEN_IF, FILE_NON_DIRECTORY_FILE, NULL, 0);
		RtlCopyMemory(str4, "    ", strlen("    "));
		size1.HighPart = 0;
		size1.QuadPart = 500;
		status = ZwWriteFile(myhandl, NULL, NULL, NULL, &BLOCK1, str4, 8, &size1, NULL);
		ZwClose(myhandl);
	}
	else
	{
		//���ļ�д��"****",
		ONOROFF = ONOROFF - 1;
		InitializeObjectAttributes(&ATTRIBUTES1, &mytext1, OBJ_CASE_INSENSITIVE, NULL, NULL);//�������Թؼ����ļ����� �����ִ�Сд  
		status = ZwCreateFile(&myhandl, GENERIC_ALL, &ATTRIBUTES1, &BLOCK1, NULL, FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ, FILE_OPEN_IF, FILE_NON_DIRECTORY_FILE, NULL, 0);
		RtlCopyMemory(str4, "****", strlen("****"));
		size1.HighPart = 0;
		size1.QuadPart = 500;
		status = ZwWriteFile(myhandl, NULL, NULL, NULL, &BLOCK1, str4, 8, &size1, NULL);
		ZwClose(myhandl);
	}

	//�ͷ��ڴ�
	ExFreePool(str4);
	ExFreePool(test);
	ExFreePool(test2);
	return STATUS_SUCCESS;
}

BOOLEAN NPUnicodeStringToChar(PUNICODE_STRING UniName, char Name[])
{
	ANSI_STRING	AnsiName;
	NTSTATUS	 ntstatus;
	char*		nameptr;

	__try {
		ntstatus = RtlUnicodeStringToAnsiString(&AnsiName, UniName, TRUE);

		if (AnsiName.Length < 260) {
			nameptr = (PCHAR)AnsiName.Buffer;
			//ת���ɴ�д�͸��Ƶ�������
			strcpy(Name, _strupr(nameptr));
			//DbgPrint("NPUnicodeStringToChar : %s\n", Name);	
		}
		RtlFreeAnsiString(&AnsiName);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint("NPUnicodeStringToChar EXCEPTION_EXECUTE_HANDLER\n");
		return FALSE;
	}
	return TRUE;
}

BOOLEAN NPUnicodeStringUpper(PUNICODE_STRING UniName, wchar_t Name[])
{
	wchar_t* nameptr;

	__try {
		
		if (UniName->Length < 260) {
			nameptr = (PCHAR)UniName->Buffer;
			//ת���ɴ�д�͸��Ƶ�������
			//strcpy(Name, _strupr(nameptr));
			wcscpy(Name, _wcsupr(nameptr));
			//DbgPrint("NPUnicodeStringUpper : %s\n", Name);	
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint("NPUnicodeStringUpper EXCEPTION_EXECUTE_HANDLER\n");
		return FALSE;
	}
	return TRUE;
}

NTSTATUS NPInstanceSetup(
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in FLT_INSTANCE_SETUP_FLAGS Flags,
	__in DEVICE_TYPE VolumeDeviceType,
	__in FLT_FILESYSTEM_TYPE VolumeFilesystemType
	)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);
	UNREFERENCED_PARAMETER(VolumeDeviceType);
	UNREFERENCED_PARAMETER(VolumeFilesystemType);

	PAGED_CODE();



	return STATUS_SUCCESS;
}


NTSTATUS
NPInstanceQueryTeardown(
__in PCFLT_RELATED_OBJECTS FltObjects,
__in FLT_INSTANCE_QUERY_TEARDOWN_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	PAGED_CODE();


	return STATUS_SUCCESS;
}


VOID NPInstanceTeardownStart(
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in FLT_INSTANCE_TEARDOWN_FLAGS Flags
	)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	PAGED_CODE();


}


VOID NPInstanceTeardownComplete(
	__in PCFLT_RELATED_OBJECTS FltObjects,
	__in FLT_INSTANCE_TEARDOWN_FLAGS Flags
	)
{
	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(Flags);

	PAGED_CODE();


}

NTSTATUS
NPUnload(
__in FLT_FILTER_UNLOAD_FLAGS Flags
)
{
	UNREFERENCED_PARAMETER(Flags);

	PAGED_CODE();


	ExFreePool(str1);
	ExFreePool(str2);
	FltCloseCommunicationPort(gServerPort);

	FltUnregisterFilter(gFilterHandle);

	return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS
NPPreCreate(
__inout PFLT_CALLBACK_DATA Data,
__in PCFLT_RELATED_OBJECTS FltObjects,
__deref_out_opt PVOID *CompletionContext
)
{
	char FileName[260] = "X:";

	NTSTATUS status;
	PFLT_FILE_NAME_INFORMATION nameInfo;

	UNREFERENCED_PARAMETER(FltObjects);
	UNREFERENCED_PARAMETER(CompletionContext);

	
	RT_TRACE(COMP_INIT, DBG_LOUD, ("NPPreCreate() \n"));
	PAGED_CODE();

	status = FltGetFileNameInformation(Data,
		FLT_FILE_NAME_NORMALIZED |
		FLT_FILE_NAME_QUERY_DEFAULT,
		&nameInfo);
	if (NT_SUCCESS(status)) {

		if (1) {
			FltParseFileNameInformation(nameInfo);
			
			if (NPUnicodeStringToChar(&nameInfo->Name, FileName)) {
				RT_TRACE(COMP_INIT, DBG_LOUD, ("NPPreCreate() %s\n", FileName));
				if (strstr(FileName, "NOTEPAD.EXE") > 0) { //��ֹ��notepad


					FltReleaseFileNameInformation(nameInfo);
					return FLT_PREOP_COMPLETE;
				}
			}
		}
		FltReleaseFileNameInformation(nameInfo);
	}

	return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}




NTSTATUS
NPMiniConnect(
__in PFLT_PORT ClientPort,
__in PVOID ServerPortCookie,
__in_bcount(SizeOfContext) PVOID ConnectionContext,
__in ULONG SizeOfContext,
__deref_out_opt PVOID *ConnectionCookie
)
{
	DbgPrint("[mini-filter] NPMiniConnect");
	PAGED_CODE();

	UNREFERENCED_PARAMETER(ServerPortCookie);
	UNREFERENCED_PARAMETER(ConnectionContext);
	UNREFERENCED_PARAMETER(SizeOfContext);
	UNREFERENCED_PARAMETER(ConnectionCookie);

	ASSERT(gClientPort == NULL);
	gClientPort = ClientPort;
	return STATUS_SUCCESS;
}

VOID
NPMiniDisconnect(
__in_opt PVOID ConnectionCookie
)
{
	PAGED_CODE();
	UNREFERENCED_PARAMETER(ConnectionCookie);
	DbgPrint("[mini-filter] NPMiniDisconnect");

	//  Close our handle
	FltCloseClientPort(gFilterHandle, &gClientPort);
}

