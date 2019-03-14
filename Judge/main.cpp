#include "JudgerType.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <windows.h>
#include <conio.h>
#include <io.h>

#include <MySql.h>
#include "Encode.h"
#include "JudgeSystem.h"

using namespace std;

struct FileData_t
{
	int RunID;
	char Data[200];

public:
	friend bool operator<(const FileData_t &a, const FileData_t &b)
	{
		return a.RunID >= b.RunID;
	}
};

void Judge(JudgeSystem_t &JudgeSystem, int RunID, const char *UserName, const char *Problem, int MaxTime, int MaxMemory, const char *Lang, const char *Test)
{
	fprintf(stdout, "运行ID:%-3d 用户:%-5s 题号:%4s 语言:%3s\n", RunID, UserName, Problem, Lang);

	int TrueTime = 0;
	int TrueMemory = 0;
	MySQL_SetOJAllRun();
	//对程序进行评测
	JudgeSystem.Remark();
	JudgeSystem.SetLanguage(Lang);
	JudgeSystem.SetLimitTime(MaxTime);
	JudgeSystem.SetLimitMemory(MaxMemory);
	char AllTest[100];
	strcpy_s(AllTest, Test);

	char *buf[100] = {};
	char *str = strtok_s(AllTest, "&", buf);

	char AllStatus[1000] = "";
	while (str)
	{
		int i = atoi(str);

		DWORD exitcode = 0;
		int Time;
		int Memory;
		int flag = JudgeSystem.StartJudge(RunID, Problem, i, exitcode, Time, Memory);
		TrueTime = max(TrueTime, Time);
		TrueMemory = max(TrueMemory, Memory);

		if (flag == CompileError)
		{
			char *Error = JudgeSystem.GetErrorData(RunID);

			fprintf(stdout, "%s", Error);

			break;
		}
		else if (flag == SystemError)
		{
			fprintf(stdout, "SystemError\n");
			break;
		}
		char iStatusStr[100];
		sprintf_s(iStatusStr, "%d&%d&%d&%d&0x%x|", i, flag, Time, Memory, exitcode);
		strcat_s(AllStatus, iStatusStr);

		fprintf(stdout, "#测试点%2d: %-18s 耗时：%-4d 退出码为0x%x\n", i, ProgramStateStr[flag], Time, exitcode);

		JudgeSystem.ContinueJudge();

		str = strtok_s(buf[0], "&", buf);
	}
	int JudgeRes = JudgeSystem.GetStatus(true);

	if (JudgeRes != CompileError && JudgeRes != SystemError)
	{
		int Strlen = (int)strnlen_s(AllStatus, 1000);
		if(Strlen >= 1)
			AllStatus[Strlen - 1] = '\0';
		MySQL_SetAllStatus(RunID, AllStatus);
		MySQL_SetUseTime(RunID, TrueTime);
		MySQL_SetUseMemory(RunID, TrueMemory);
	}

	fprintf(stdout, "结果：");
	if (JudgeRes == Running)
	{
		fprintf(stdout, "Accepted, 耗时：%d, 代码长度:%d\n\n", TrueTime, JudgeSystem.GetCodeLen());
		MySQL_ChangeStatus(RunID, "Accepted");
	}
	else
	{
		fprintf(stdout, "%s, 耗时：%d, 代码长度:%d\n\n", ProgramStateStr[JudgeRes], TrueTime, JudgeSystem.GetCodeLen());
		MySQL_ChangeStatus(RunID, ProgramStateStr[JudgeRes]);
	}

	Sleep(1000);
	JudgeSystem.DeleteDir(RunID);
}

//获取数据文件中的信息
void FileToData(const char *FileData, char *Problem, int &MaxTime, int &MaxMemory, char *UserName, char *Lang, char *Test)
{
	char iData[200];
	strcpy_s(iData, FileData);

	char *Context[100] = {};
	//语言
	char *Data = strtok_s(iData, "|", Context);
	strcpy_s(Lang, 100, Data);
	//用户名
	Data = strtok_s(Context[0], "|", Context);
	strcpy_s(UserName, 100, Data);
	//题号
	Data = strtok_s(Context[0], "|", Context);
	strcpy_s(Problem, 100, Data);
	//最大时间
	Data = strtok_s(Context[0], "|", Context);
	MaxTime = atoi(Data);
	//最大内存
	Data = strtok_s(Context[0], "|", Context);
	MaxMemory = atoi(Data);
	//测试点
	Data = strtok_s(Context[0], "|", Context);
	strcpy_s(Test, 100, Data);
}


void getFiles(const char *path, priority_queue<FileData_t> &files)
{
	intptr_t hFile = 0;
	_finddata_t fileinfo;
	string p;

	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			if (!(fileinfo.attrib & _A_SUBDIR))
			{
				FileData_t item;
				item.RunID = atoi(fileinfo.name);

				char FilePath[100];
				sprintf_s(FilePath, "%s\\%s", path, fileinfo.name);

				char buf[100];
				ifstream is(FilePath);
				is.getline(buf, 200);
				is.close();
				remove(FilePath);

				wchar_t *wBuf = UTF8ToUnicode(buf);
				char *AnsiBuf = UnicodeToANSI(wBuf);
				strcpy_s(item.Data, AnsiBuf);

				files.push(item);

				MySQL_ChangeStatus(item.RunID, "Queuing");
			}
		} while (_findnext(hFile, &fileinfo) == 0);

		_findclose(hFile);
	}
}

BOOL WINAPI CloseJudge(DWORD dwCtrlType)
{
	MySQL_SetOJState(0);
	MySQL_Delete();

	return TRUE;
}

int main()
{
	JudgeSystem_t JudgeSystem;
	char filePath[100];
#ifndef JUDGE_CONTEST
	strcpy_s(filePath, "Temporary_Data");
#endif
#ifdef JUDGE_CONTEST
	strcpy_s(filePath, "Temporary_ContestData");
#endif

	MySQL_Connect();
	MySQL_SetOJState(1);
	printf("%s ： 开始评测\n", MySQL_GetOJName());

	SetConsoleCtrlHandler(CloseJudge, TRUE);

	priority_queue<FileData_t> coData;

	while (true)
	{
		getFiles(filePath, coData);

		while (coData.size() != 0)
		{
			//Judge
			int RunID = coData.top().RunID;

			char LogPath[100];
			sprintf_s(LogPath, "log\\Judge_%d.log", RunID);
			FILE *stream;
			freopen_s(&stream, LogPath, "w", stdout);

			fprintf(stdout, "%s\n", coData.top().Data);

			//问题
			char Problem[100];
			//用户名
			char UserName[100];
			//语言
			char Lang[100];
			//测试点
			char Test[100];
			//最大时间
			int MaxTime;
			//最大内存
			int MaxMemory;
			FileToData(coData.top().Data, Problem, MaxTime, MaxMemory, UserName, Lang, Test);


			char Path1[100], Path2[100];
			if (!strcmp(Lang, "Java"))
			{
				sprintf_s(Path1, "Temporary_Code\\%d", RunID);
				sprintf_s(Path2, "test\\%d\\Main.java", RunID);
			}
			else
			{
				sprintf_s(Path1, "Temporary_Code\\%d", RunID);
				sprintf_s(Path2, "test\\%d\\Code.cpp", RunID);
			}

			JudgeSystem.CreateDir(RunID);

			CopyFile(Path1, Path2, TRUE);

			Judge(JudgeSystem, RunID, UserName, Problem, MaxTime, MaxMemory, Lang, Test);
			coData.pop();

			fclose(stream);
		}

		Sleep(1000);
	}

	MySQL_SetOJState(0);
	MySQL_Delete();

	system("pause");
	return 0;
}
