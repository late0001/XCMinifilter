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
__in PDRIVER_OBJECT DriverObject,  //PDRIVER_OBJECT 驱动数据结构（驱动对象）的指针
__in PUNICODE_STRING RegistryPath//PUNICODE_STRING 内核字符串数组，驱动以服务的形式加载，这个字符串为驱动在注册表的路径
)
{
	NTSTATUS status;              //返回值状态
	PSECURITY_DESCRIPTOR sd;     //安全描述符。
	OBJECT_ATTRIBUTES oa;        //对象属性
	UNICODE_STRING uniString;		//用于通信端口名称

	UNREFERENCED_PARAMETER(RegistryPath);//避免编译器关于未引用参数的警告
	DriverObject->DriverUnload = DriverUnload;
	status = STATUS_SUCCESS;
	RT_TRACE(COMP_INIT, DBG_LOUD, ("DriverEntry ==>\n"));
	//return status;
#if 1
	// 注册FilterRegistration，告诉系统我设置的回调例程
	//第1个参数是本驱动的驱动对象，是在入口函数DriverEntry 中作为参数传入的。
	//第2个参数就是一个注册信息的结构，这个结构内含描述这个过滤器的所有信息
	//第3个参数 是一个返回参数，返回注册成功的微过滤器句柄，在下面调用函数FltStartFiltering时会用到
	//DbgPrint("statuswww=\n");
	status = FltRegisterFilter(DriverObject,
		&FilterRegistration,
		&gFilterHandle);

	ASSERT(NT_SUCCESS(status));//检查结果是否成功，失败后触发异常被调试器接管

	if (NT_SUCCESS(status)) {//检查结果是否成功，

		//
		//  开始函数，只有一个参数为之前获取的句柄
		//

		status = FltStartFiltering(gFilterHandle);

		if (!NT_SUCCESS(status)) {
			//如果失败就取消注册，只有一个参数为之前获取的句柄
			FltUnregisterFilter(gFilterHandle);
		}
	}

	//FltBuildDefaultSecurityDescriptor构建一个默认的安全描述符，用于FltCreateCommunicationPort。
	//参数1指向调用方分配的变量的指针，该变量接收到指向新创建的不透明指针
	//参数2指定调用者需要对端口对象的访问类型的标志的位掩码。系统定义的DesiredAccess标志集确定了minifilter驱动程序通信端口对象的以下特定访问权限。
	status = FltBuildDefaultSecurityDescriptor(&sd, FLT_PORT_ALL_ACCESS);

	if (!NT_SUCCESS(status)) {//检查结果是否成功，未成功就跳向最终处理部分
		goto final;
	}

	//函数初始化Unicode字符的计数字符串
	RtlInitUnicodeString(&uniString, MINISPY_PORT_NAME);

	//宏初始化不透明的OBJECT_ATTRIBUTES结构，该结构将对象句柄的属性指定给打开句柄的例程。
	//参数1为要被初始化的结构体，参数2一个指向Unicode字符串的指针，该字符串包含要为其打开句柄的对象的名称。
	//这必须是一个完全限定的对象名，或者由RootDirectory参数指定的对象目录的相对路径名。
	//参数3标志位，此处指定为只在内核模式访问，与对大小写不区分
	//参数4在ObjectName参数中指定的路径名的根对象目录句柄。如果ObjectName是完全限定的对象名称，
	//则RootDirectory为空。使用ZwCreateDirectoryObject获得对象目录的句柄。
	//参数5指定创建对象时应用于该对象的安全描述符。此参数是可选的。驱动程序可以指定NULL来接受对象的默认安全性。
	InitializeObjectAttributes(&oa,
		&uniString,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL,
		sd);

	//创建一个通信服务器端口，minifilter驱动程序可以在该端口上接收来自用户模式应用程序的连接请求。
	// 参数1调用者的不透明过滤器指针
	//参数2指向调用方分配的变量的指针，该变量接收通信服务器端口的不透明端口句柄。minifilter驱动程序使用这个句柄来监听来自用户模式应用程序的连接请求。
	//参数3指向OBJECT_ATTRIBUTES结构的指针，该结构指定通信服务器端口的属性。
	//此结构必须由之前的InitializeObjectAttributes调用初始化。此参数是必需的，不能为空。通信端口对象的此结构的成员包括以下内容。
	//参数4指向由minifilter驱动程序定义的上下文信息的指针。这个信息可以用来区分由相同的minifilter驱动程序创建的多个通信服务器端口。
	//过滤器管理器将这个上下文指针作为参数传递给ConnectNotifyCallback例程。该参数是可选的，可以为NULL。
	//参数5 NPMiniConneet是用户态与内核态建立连接时内核会调用到的函数。
	//参数6 NPMiniDisconnect 是用户态与内核态连接结束时内核会调用到的函数。。
	//参数7 NPMiniMessage是用户态与内核态传送数据时内核会调用到的函数
	//参数8此服务器端口所允许的最大并发客户端连接数。此参数是必需的，且必须大于零。
	status = FltCreateCommunicationPort(gFilterHandle,
		&gServerPort,
		&oa,
		NULL,
		NPMiniConnect,
		NPMiniDisconnect,
		NPMiniMessage,
		1);
	//释放分配的安全描述符
	FltFreeSecurityDescriptor(sd);

	if (!NT_SUCCESS(status)) {
		goto final;
	}

	// 返回值

	// 首先初始化含有文件路径的OBJECT_ATTRIBU
	UNICODE_STRING mytext1;  //文件名字符串
	OBJECT_ATTRIBUTES ATTRIBUTES1;
	IO_STATUS_BLOCK BLOCK1;//可拥有的控制权  
	LARGE_INTEGER size1;  //写的偏移大小

	str1 = ExAllocatePool(NonPagedPool, 500);//分配500的非分页内存
	RtlZeroMemory(str1, 500);//清0内存

	RtlInitUnicodeString(&mytext1, L"\\??\\C:\\true.txt");//初始化文件名  
	memset(&ATTRIBUTES1, 0, sizeof(OBJECT_ATTRIBUTES));                        //对象属性清空  
	size1.QuadPart = 0;

	InitializeObjectAttributes(&ATTRIBUTES1, &mytext1, OBJ_CASE_INSENSITIVE, NULL, NULL);//对象属性关键是文件名字 不区分大小写 ，参数意义同上

	//打开目标文件获取句柄
	//参数1：一个指向句柄变量的指针，该句柄用于接收文件的句柄。
	//参数2：指定ACCESS_MASK值，该值确定请求对对象的访问。除了为所有类型的对象定义的访问权限之外，调用者还可以指定以下特定于文件的访问权限。
	//参数3：OBJECT_ATTRIBUTES结构的指针，该结构指定对象名和其他属性。使用InitializeObjectAttributes初始化该结构。
	//参数4：用于接收最终完成状态和请求操作的其他信息
	//参数5：它包含创建或覆盖的文件的初始分配大小，可以为Null
	//参数6：表示创建或覆盖文件时要设置的文件属性。调用者通常指定FILE_ATTRIBUTE_NORMAL，它设置默认属性
	//参数7：共享访问的类型，
	//参数8：指定在文件存在或不存在时执行的操作
	//参数9：指定驱动程序创建或打开文件时应用的选项
	//参数10：对于设备和中间驱动程序，此参数必须为空指针
	//参数11：对于设备和中间驱动程序，此参数必须为零。
	status = ZwCreateFile(&myhandl, GENERIC_ALL, &ATTRIBUTES1, &BLOCK1, NULL, FILE_ATTRIBUTE_NORMAL,
		FILE_SHARE_READ, FILE_OPEN_IF, FILE_NON_DIRECTORY_FILE, NULL, 0);

	//通过句柄读取目标文件放入str1
	if (!NT_SUCCESS(status))
	{
		FirsT = 0;
		goto final;
	}
	else
	{
		status = ZwReadFile(myhandl, NULL, NULL, NULL, &BLOCK1, str1, 500, &size1, NULL);
		FirsT = 1;//防止重入
		DbgPrint("status=%X\n", status);

	}

	//以下内容基本同上只是文件的对象不一样
	UNICODE_STRING mytext2;
	OBJECT_ATTRIBUTES ATTRIBUTES2;
	IO_STATUS_BLOCK BLOCK2;//可拥有的控制权  
	LARGE_INTEGER size2;
	str2 = ExAllocatePool(NonPagedPool, 500);
	RtlZeroMemory(str2, 500);
	RtlInitUnicodeString(&mytext2, L"\\??\\C:\\false.txt");//初始化文件名  
	memset(&ATTRIBUTES2, 0, sizeof(OBJECT_ATTRIBUTES));                        //对象属性清空  
	size2.QuadPart = 0;
	InitializeObjectAttributes(&ATTRIBUTES2, &mytext2, OBJ_CASE_INSENSITIVE, NULL, NULL);//对象属性关键是文件名字 不区分大小写  
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



	//对目标文件true.txt进行写入，从而触发read
	PVOID str3 = ExAllocatePool(PagedPool, 10);
	RtlZeroMemory(str3, 6);
	RtlCopyMemory(str3, "...", strlen("..."));
	size1.HighPart = 0;
	size1.QuadPart = 500;
	status = ZwWriteFile(myhandl, NULL, NULL, NULL, &BLOCK1, str3, 6, &size1, NULL);


	//关闭句柄
	ZwClose(myhandl);
	ZwClose(myhand2);
#endif
	final ://失败处理处

	if (!NT_SUCCESS(status)) {

		if (NULL != gServerPort) {
			//关闭过滤器驱动程序的通信服务器端口
			FltCloseCommunicationPort(gServerPort);
		}

		if (NULL != gFilterHandle) {
			//取消注册
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
	char FileName[260] = "X:";//存放解析后的文件名字
	wchar_t wFileName[260] = L"X:";//存放解析后的文件名字
	NTSTATUS status;  //返回值状态
	PFLT_FILE_NAME_INFORMATION nameInfo; //名称信息
	RT_TRACE(COMP_INIT, DBG_LOUD, ("MyPostRead ==>\n"));
	//
	// 对失败的irp进行过滤，如果失败了，就不用继续
	if (!NT_SUCCESS(Data->IoStatus.Status) ||
		(STATUS_REPARSE == Data->IoStatus.Status)) {

		return FLT_POSTOP_FINISHED_PROCESSING;
	}



	//FltGetFileNameInformation例程返回文件或目录的名称信息。               								
	status = FltGetFileNameInformation(Data,//PFLT_CALLBACK_DATA结构的指针，它是I/O操作的回调数据结构。此参数是必需的，不能为空。
		FLT_FILE_NAME_NORMALIZED |//使nameInfo参数接收包含文件规范化名称的结构的地址。
		FLT_FILE_NAME_QUERY_DEFAULT,//如果查询文件系统以获取文件名目前还不安全，则FltGetFileNameInformation将不执行任何操作。
		//否则，FltGetFileNameInformation查询过滤器管理器的名称缓存以获取文件名信息。
		//如果在缓存中没有找到该名称，则FltGetFileNameInformation查询文件系统并缓存结果。
		&nameInfo);//包含文件名信息。

	if (NT_SUCCESS(status))
	{
		//解析FLT_FILE_NAME_INFORMATION结构的内容。
		status = FltParseFileNameInformation(nameInfo);


		//参数1大小写转换，后放入参数2
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
					//如果为mdl方式读取文件，就获取地址
					buf1 = MmGetSystemAddressForMdlSafe(Data->Iopb->Parameters.Read.MdlAddress, NormalPagePriority);

				}
				else
				{
					//如果为ReadBuffer就直接给指针
					buf1 = Data->Iopb->Parameters.Read.ReadBuffer;
				}
				//判断应用层发来的指令是否为显示true.txt
				if (command == 2)
				{
					if (FirsT == 1)
					{
						strcpy(str1, "1111111111111");
						//复制内存，填写长度
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
						//复制内存，填写长度
						RtlCopyMemory(buf1, str2, 500);
						Data->IoStatus.Information = Data->Iopb->Parameters.Read.Length;
					}

				}
				DbgPrint("ReadBuffed=%s", buf1);
				//释放文件名信息结构体的内存
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
	//写入文件需要的各种参数，意义同dirverEntry
	NTSTATUS status;
	UNICODE_STRING mytext1;
	OBJECT_ATTRIBUTES ATTRIBUTES1;
	IO_STATUS_BLOCK BLOCK1;//可拥有的控制权  
	LARGE_INTEGER size1;
	PVOID str4 = ExAllocatePool(PagedPool, 40);

	RT_TRACE(COMP_INIT, DBG_LOUD, ("NPMiniMessage ==>\n"));
	RtlZeroMemory(str4, 40);
	RtlInitUnicodeString(&mytext1, L"\\??\\C:\\true.txt");//初始化文件名  
	memset(&ATTRIBUTES1, 0, sizeof(OBJECT_ATTRIBUTES));                        //对象属性清空  
	size1.QuadPart = 0;


	PAGED_CODE();
	//分配内存，存储字符串，用于和应用层字符串比较
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
	//通过应用层输入的字符串，来显示相应的文本
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

	//向文件写入，来触发read，由于需要内容不同，所以通过改变ONOROFF，实现两种写入依次执行
	if (ONOROFF == 0)
	{
		//向文件写入空格
		ONOROFF = ONOROFF + 1;
		InitializeObjectAttributes(&ATTRIBUTES1, &mytext1, OBJ_CASE_INSENSITIVE, NULL, NULL);//对象属性关键是文件名字 不区分大小写  
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
		//向文件写入"****",
		ONOROFF = ONOROFF - 1;
		InitializeObjectAttributes(&ATTRIBUTES1, &mytext1, OBJ_CASE_INSENSITIVE, NULL, NULL);//对象属性关键是文件名字 不区分大小写  
		status = ZwCreateFile(&myhandl, GENERIC_ALL, &ATTRIBUTES1, &BLOCK1, NULL, FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_READ, FILE_OPEN_IF, FILE_NON_DIRECTORY_FILE, NULL, 0);
		RtlCopyMemory(str4, "****", strlen("****"));
		size1.HighPart = 0;
		size1.QuadPart = 500;
		status = ZwWriteFile(myhandl, NULL, NULL, NULL, &BLOCK1, str4, 8, &size1, NULL);
		ZwClose(myhandl);
	}

	//释放内存
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
			//转换成大写和复制到缓冲区
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
			//转换成大写和复制到缓冲区
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
				if (strstr(FileName, "NOTEPAD.EXE") > 0) { //禁止打开notepad


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

