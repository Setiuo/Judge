#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <iostream>
#include <queue>
#include <string>
#include <fstream>
#include <time.h>
#include <Psapi.h>

#include "JudgeSystem.h"
#include "MySql.h"
#include "Encode.h"

using namespace std;

JudgeSystem_t::JudgeSystem_t()
{
	this->CodeLen = 0;
	strcpy_s(this->LanguageType, "g++");
	this->status = Queuing;
	this->JudgeStatus = Queuing;
	this->HaveCompile = false;
}

JudgeSystem_t::~JudgeSystem_t()
{

}

void JudgeSystem_t::SetLanguage(const char *cmp)
{
	strcpy_s(LanguageType, cmp);
}

char* JudgeSystem_t::GettLanguage()
{
	return this->LanguageType;
}

void JudgeSystem_t::Remark()
{
	this->CodeLen = 0;
	this->status = Queuing;
	this->JudgeStatus = Queuing;
	this->HaveCompile = false;
}

void JudgeSystem_t::ContinueJudge()
{
	this->status = Running;
}

int JudgeSystem_t::GetStatus(bool type = false)
{
	if (!type)
		return this->status;
	else
		return this->JudgeStatus;
}

bool JudgeSystem_t::CheckCode(int RunID)
{
	bool PassStatus = true;

	char CodePath[PathLen];
	sprintf_s(CodePath, "test\\%d\\Code.cpp", RunID);
	const string SensitiveStr[] = {"GetCurrentProcess", "system", "ExitWindowsEx", "fopen", "ifstream", "WinExec", "ShellExecute", "WSAStartup", "ShellExecuteEx", "CreateProcess", "CreateThread", "OpenProcess", "CloseHandle", "ShellExecuteW", "ShellExecuteA", "remove", "DeleteFileW", "DeleteFileA", "CreateFileW", "CreateFileA","DeleteFile", "CreateDirectory", "MessageBox" , "MessageBoxA", "MessageBoxW", "accept", "socket", "GetCurrentProcessId", "TerminateProcess"};
	int SensitiveStrNum = sizeof(SensitiveStr) / sizeof(SensitiveStr[0]);
	string Code;

	ifstream infile;
	infile.open(CodePath); 
	string Str;

	while (getline(infile, Str))
	{
		for (int i = 0; i < SensitiveStrNum; i++)
		{
			if (Str.find(SensitiveStr[i]) != string::npos)
			{
				PassStatus = false;
				this->status = CompileError;

				sprintf_s(CodePath, "Temporary_Error\\%d.log", RunID);
				ofstream out(CodePath);
				if (out.is_open())
				{
					out << UnicodeToUTF8(L"检测到敏感词: ") << SensitiveStr[i].c_str() << endl;
					out << UnicodeToUTF8(L"编译已终止，请删去敏感词再进行操作.");
					out.close();
				}

				break;
			}
		}

		if (!PassStatus)
		{
			break;
		}
	}
	infile.close();

	return PassStatus;
}
int JudgeSystem_t::StartJudge(int RunID, const char *Problem, int TestID, DWORD &exitcode, int &Time, int &Memory)
{
	if (!this->HaveCompile)
	{
		this->JudgeStatus = Compiling;
		MySQL_ChangeStatus(RunID, "Compiling");

		this->CreateDir(RunID);

		if (!strcmp(GettLanguage(), "Java"))
		{
			if (this->Compiler_JAVA(RunID))
			{
				this->HaveCompile = true;
				this->JudgeStatus = Running;
				MySQL_ChangeStatus(RunID, "Running");
			}
			else
			{
				this->HaveCompile = false;

				if (this->JudgeStatus == Compiling)
				{
					this->JudgeStatus = CompileError;
					MySQL_ChangeStatus(RunID, "Compile Error");
				}

			}
		}
		else
		{
			if (this->CheckCode(RunID) && this->Compiler_C(RunID))
			{
				this->HaveCompile = true;
				this->JudgeStatus = Running;
				MySQL_ChangeStatus(RunID, "Running");
			}
			else
			{
				this->HaveCompile = false;

				if (this->JudgeStatus == Compiling)
				{
					this->JudgeStatus = CompileError;
					MySQL_ChangeStatus(RunID, "Compile Error");
				}

			}
		}
	}

	if (this->HaveCompile)
	{
		this->Judge(RunID, Problem, TestID, exitcode, Time, Memory);
	}

	return this->status;
}

//AC结果评测
bool JudgeSystem_t::AcceptedTest(const char* program, const char* tester)
{
	ifstream is1(program);
	ifstream is2(tester);

	bool Res = true;

	char buf1[1000];
	char buf2[1000];

	while (is2.getline(buf2, 1000))
	{
		if (!is1.getline(buf1, 1000))
		{
			Res = false;
			break;
		}

		if (strcmp(buf1, buf2) != 0)
		{
			Res = false;
			break;
		}
	}

	if (is1.getline(buf1, 1000))
	{
		Res = false;
	}

	is1.close();
	is2.close();

	return Res;
}

//PE结果评测
bool JudgeSystem_t::PresentationErrorTest(const char* program, const char* tester)
{
	ifstream is1(program);
	ifstream is2(tester);

	char buf1[1000];
	char buf2[1000];

	bool Res = true;

	while (is2 >> buf2)
	{
		if (!(is1 >> buf1))
		{
			Res = false;
			break;
		}

		if (strcmp(buf1, buf2) != 0)
		{
			Res = false;
			break;
		}
	}

	if (is1 >> buf1)
	{
		Res = false;
	}

	is1.close();
	is2.close();

	return Res;
}

bool JudgeSystem_t::OutputLimitExceededTest(const char* program, const char* tester)
{
	ifstream is1(program);
	ifstream is2(tester);

	char buf1[1000];
	char buf2[1000];

	bool Res = true;

	while (is1 >> buf1)
	{
		if (!(is2 >> buf2))
		{
			Res = false;
			break;
		}
	}

	if (!(is2 >> buf2) && Res)
	{
		Res = false;
	}

	is1.close();
	is2.close();

	return Res;
}

//编译JAVA文件
bool JudgeSystem_t::Compiler_JAVA(int RunID)
{
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;

	char PutPath[PathLen];
	sprintf_s(PutPath, "./Temporary_Error/%d.log", RunID);
	HANDLE cmdError = CreateFile(PutPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	char CodePath[PathLen];
	sprintf_s(CodePath, "test\\%d\\Main.java", RunID);
	//执行的编译命令
	char command[PathLen];
	sprintf_s(command, "javac \"test\\%d\\Main.java\"", RunID);

	//状态
	this->status = Compiling;

	//重定向编译错误输出
	si.hStdError = cmdError;
	si.dwFlags = STARTF_USESTDHANDLES;

	DWORD exitcode;
	//创建进程
	if (CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		printf("编译中...\n");
		WaitForSingleObject(pi.hProcess, INFINITE);

		GetExitCodeProcess(pi.hProcess, &exitcode);

		if (exitcode != 0)
		{
			this->status = CompileError;
			printf("编译发生错误\n");
		}
		else
		{
			this->status = Running;
			printf("编译完成\n");
		}
	}
	else
	{
		this->JudgeStatus = SystemError;
		this->status = SystemError;
	}

	TerminateThread(pi.hThread, 0);
	TerminateProcess(pi.hProcess, 0);

	CloseHandle(cmdError);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	//获取代码长度
	ifstream is;
	is.open(CodePath, ios::binary);
	is.seekg(0, ios::end);
	this->CodeLen = (long)is.tellg();
	is.close();

	if (this->status == Running)
		return true;
	else
		return false;
}

//编译c++文件
bool JudgeSystem_t::Compiler_C(int RunID)
{
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;

	char PutPath[PathLen];
	sprintf_s(PutPath, "./Temporary_Error/%d.log", RunID);
	HANDLE cmdError = CreateFile(PutPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	char CodePath[PathLen];
	sprintf_s(CodePath, "test\\%d\\Code.cpp", RunID);
	//执行的编译命令
	char command[PathLen];
	sprintf_s(command, "\"%s\\%s.exe\" \"%s\" -o \"test\\%d\\Test.exe\" -ansi -fno-asm -O2 -Wall -lm --static -DONLINE_JUDGE", CompilerPath, LanguageType, CodePath, RunID);

	//状态
	this->status = Compiling;

	//重定向编译错误输出
	si.hStdError = cmdError;
	si.dwFlags = STARTF_USESTDHANDLES;

	DWORD exitcode;
	//创建进程
	if (CreateProcess(NULL, command, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		printf("编译中...\n");
		WaitForSingleObject(pi.hProcess, INFINITE);

		//sprintf_s(PutPath, "test/%d/Test.exe", RunID);
		GetExitCodeProcess(pi.hProcess, &exitcode);

		if (exitcode != 0)
		{
			this->status = CompileError;
			printf("编译发生错误\n");
		}
		else
		{
			this->status = Running;
			printf("编译完成\n");
		}
	}
	else
	{
		this->JudgeStatus = SystemError;
		this->status = SystemError;
	}

	TerminateThread(pi.hThread, 0);
	TerminateProcess(pi.hProcess, 0);

	CloseHandle(cmdError);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	//获取代码长度
	ifstream is;
	is.open(CodePath, ios::binary);
	is.seekg(0, ios::end);
	this->CodeLen = (long)is.tellg();
	is.close();

	if (this->status == Running)
		return true;
	else
		return false;
}


//删除文件及文件夹，节省空间
void JudgeSystem_t::DeleteDir(int RunID)
{
	char Path[PathLen];
	//删除目录下所有文件
	sprintf_s(Path, "del /s /q \"test\\%d\\*.*\" >log\\Remove_%d.log", RunID, RunID);
	system(Path);
	//删除文件夹
	sprintf_s(Path, "rd \"test\\%d\"", RunID);
	system(Path);
}

//创建文件夹
void JudgeSystem_t::CreateDir(int RunID)
{
	char Path[PathLen];
	sprintf_s(Path, "test/%d", RunID);

	CreateDirectory(Path, NULL);
}

void JudgeSystem_t::SetLimitTime(int MaxTime)
{
	this->MaxTime = MaxTime;
}

void JudgeSystem_t::SetLimitMemory(int MaxMemory)
{
	this->MaxMemory = MaxMemory;
}

//评测
void JudgeSystem_t::Judge(int RunID, const char *Problem, int TestID, DWORD &exitcode, int &Time, int &Memory)
{
	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;

	//输出文件路径
	char OutPutPath[PathLen];
	sprintf_s(OutPutPath, "./test/%d/Test%d.out", RunID, TestID);
	//重定向std输出
	HANDLE cmdOutput = CreateFile(OutPutPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (cmdOutput == INVALID_HANDLE_VALUE)
	{
		exitcode = 233;
		this->status = RuntimeError;
		return;
		//ExitProcess(0);
	}
	//测试点输入路径
	char PutPath[PathLen];
	sprintf_s(PutPath, "./data/%s/%s_%d.in", Problem, Problem, TestID);
	//重定向std输入
	HANDLE cmdInput = CreateFile(PutPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, &sa, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (cmdInput == INVALID_HANDLE_VALUE)
	{
		exitcode = 233;
		this->status = RuntimeError;
		return;
		//ExitProcess(0);
	}

	si.hStdInput = cmdInput;
	si.hStdOutput = cmdOutput;
	si.dwFlags = STARTF_USESTDHANDLES;

	//编译出的程序路径
	char ExePath[PathLen];
	if (!strcmp(GettLanguage(), "Java"))
	{
		sprintf_s(ExePath, "\"java\" -cp \"test\\%d\" \"Main\"", RunID);
	}
	else
	{
		sprintf_s(ExePath, "./test/%d/Test.exe", RunID);
	}

	//创建进程进行测试
	if (CreateProcess(NULL, ExePath, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
		//int cur_time1 = clock();
		//int iTime = 0;
		bool iTimeLimitExceeded = true;

		Time = 0;
		//这样写的目的是确保程序的结束不是在运行异常的情况下
		while (Time <= MaxTime)
		{
			FILETIME creationTime, exitTime, kernelTime, userTime;
			SYSTEMTIME realTime;

			DWORD WaitType = WaitForSingleObject(pi.hProcess, MaxTime);
			
			GetProcessTimes(pi.hProcess, &creationTime, &exitTime, &kernelTime, &userTime);
			FileTimeToSystemTime(&userTime, &realTime);

			Time = realTime.wMilliseconds
				+ realTime.wSecond * 1000
				+ realTime.wMinute * 60 * 1000
				+ realTime.wHour * 60 * 60 * 1000;

			if (WaitType == WAIT_OBJECT_0)
			{
				if (Time <= MaxTime)
				{
					iTimeLimitExceeded = false;
				}
				
				break;
			}
			printf("程序发生异常，正在重测...\n");
		}
		//等待进程结束
		//WaitForSingleObject(pi.hProcess, MaxTime);
		GetExitCodeProcess(pi.hProcess, &exitcode);


		PROCESS_MEMORY_COUNTERS_EX info;
		ZeroMemory(&info, sizeof(info));
		info.cb = sizeof(info);

		GetProcessMemoryInfo(pi.hProcess, (PROCESS_MEMORY_COUNTERS*)&info, sizeof(info));
		Memory = (int)info.PeakWorkingSetSize / 1024;

		switch (exitcode)
		{
		case 1:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:		//整数除法的除数是0时引发该异常。
		case EXCEPTION_INT_OVERFLOW:			//整数操作的结果溢出时引发该异常。
		case EXCEPTION_ACCESS_VIOLATION:		//程序企图读写一个不可访问的地址时引发的异常。例如企图读取0地址处的内存。
		case EXCEPTION_DATATYPE_MISALIGNMENT:	//程序读取一个未经对齐的数据时引发的异常。
		case EXCEPTION_FLT_STACK_CHECK:			//进行浮点数运算时栈发生溢出或下溢时引发该异常。
		case EXCEPTION_INVALID_DISPOSITION:		//异常处理器返回一个无效的处理的时引发该异常。
		case EXCEPTION_STACK_OVERFLOW:			//栈溢出时引发该异常。
		{
			this->status = RuntimeError;
			this->JudgeStatus = RuntimeError;
			break;
		}
		default:
			break;
		}

		
		TerminateThread(pi.hThread, 0);
		TerminateProcess(pi.hProcess, 0);
		TerminateJobObject(pi.hProcess, 0);

		if (Memory > MaxMemory && this->status != RuntimeError)
		{
			this->status = MemoryLimitExceeded;

			if (this->JudgeStatus == Running)
			{
				this->JudgeStatus = MemoryLimitExceeded;
			}
		}

		if (iTimeLimitExceeded && this->status == Running)
		{
			this->status = TimeLimitExceeded;

			if (this->JudgeStatus == Running)
			{
				this->JudgeStatus = TimeLimitExceeded;
			}
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	else
	{
		this->status = SystemError;
		this->JudgeStatus = SystemError;
		MySQL_ChangeStatus(RunID, "System Error");
	}

	CloseHandle(cmdOutput);
	CloseHandle(cmdInput);

	if (this->status == Running)
	{
		char ProPutPath[PathLen];
		sprintf_s(ProPutPath, "./data/%s/%s_%d.out", Problem, Problem, TestID);

		if (AcceptedTest(ProPutPath, OutPutPath))
		{
			this->status = Accepted;
		}
		else if(PresentationErrorTest(ProPutPath, OutPutPath))
		{
			this->status = PresentationError;

			if (this->JudgeStatus == Running)
			{
				this->JudgeStatus = PresentationError;
			}
		}
		else if (OutputLimitExceededTest(ProPutPath, OutPutPath))
		{
			this->status = OutputLimitExceeded;

			if (this->JudgeStatus == Running)
			{
				this->JudgeStatus = OutputLimitExceeded;
			}
		}
		else
		{
			this->status = WrongAnswer;

			if (this->JudgeStatus == Running || this->JudgeStatus == TimeLimitExceeded || this->JudgeStatus == PresentationError || this->JudgeStatus == MemoryLimitExceeded)
			{
				this->JudgeStatus = WrongAnswer;
			}
		}
	}
}

char* JudgeSystem_t::GetErrorData(int RunID)
{
	char PutPath[PathLen];
	sprintf_s(PutPath, "./Temporary_Error/%d.log", RunID);

	static char Error[1024] = "";
	strcpy_s(Error, "");

	ifstream is(PutPath);

	char buf[1000];

	while (is.getline(buf, 1000))
	{
		wchar_t *wBuf = UTF8ToUnicode(buf);
		char *AnsiBuf = UnicodeToANSI(wBuf);

		strcat_s(Error, AnsiBuf);
		strcat_s(Error, "\n");
	}

	is.close();
	return Error;
}

long JudgeSystem_t::GetCodeLen()
{
	return this->CodeLen;
}