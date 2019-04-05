#include "JudgerType.h"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <queue>
#include <windows.h>
#include <conio.h>
#include <io.h>
#include <time.h>

#include <MySql.h>
#include "Encode.h"
#include "JudgeSystem.h"

using namespace std;

struct FileData_t
{
	int RunID;
	char Data[500];

public:
	friend bool operator<(const FileData_t &a, const FileData_t &b)
	{
		return a.RunID >= b.RunID;
	}
};

void Judge(JudgeSystem_t &JudgeSystem, int RunID, const char *UserName, const char *Problem, int MaxTime, int MaxMemory, const char *Lang, const char *Test)
{
	time_t tt = time(NULL);//��䷵�ص�ֻ��һ��ʱ��cuo
	tm* t = new tm;
	localtime_s(t, &tt);

	fprintf(stdout, "����ID:%-3d �û�:%-5s ���:%4s ����:%3s\n", RunID, UserName, Problem, Lang);
	fprintf(stdout, "%d-%02d-%02d %02d:%02d:%02d\n",t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	delete t;

	int TrueTime = 0;
	int TrueMemory = 0;
	MySQL_SetOJAllRun();
	//�Գ����������
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

		fprintf(stdout, "#���Ե�%2d: %-18s ��ʱ��%-4d �˳���Ϊ0x%x\n", i, ProgramStateStr[flag], Time, exitcode);

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

	fprintf(stdout, "�����");
	if (JudgeRes == Running)
	{
		fprintf(stdout, "Accepted, ��ʱ��%d, ���볤��:%d\n\n", TrueTime, JudgeSystem.GetCodeLen());
		MySQL_ChangeStatus(RunID, ProgramStateStr[Accepted]);
	}
	else
	{
		fprintf(stdout, "%s, ��ʱ��%d, ���볤��:%d\n\n", ProgramStateStr[JudgeRes], TrueTime, JudgeSystem.GetCodeLen());
		MySQL_ChangeStatus(RunID, ProgramStateStr[JudgeRes]);
	}

	Sleep(1000);
	JudgeSystem.DeleteDir(RunID);
}

//��ȡ�����ļ��е���Ϣ
void FileToData(const char *FileData, char *Problem, int &MaxTime, int &MaxMemory, char *UserName, char *Lang, char *Test)
{
	char iData[500];
	strcpy_s(iData, FileData);

	char *Context[500] = {};
	//����
	char *Data = strtok_s(iData, "|", Context);
	strcpy_s(Lang, 100, Data);
	//�û���
	Data = strtok_s(Context[0], "|", Context);
	strcpy_s(UserName, 100, Data);
	//���
	Data = strtok_s(Context[0], "|", Context);
	strcpy_s(Problem, 100, Data);
	//���ʱ��
	Data = strtok_s(Context[0], "|", Context);
	MaxTime = atoi(Data);
	//����ڴ�
	Data = strtok_s(Context[0], "|", Context);
	MaxMemory = atoi(Data);
	//���Ե�
	Data = strtok_s(Context[0], "|", Context);
	strcpy_s(Test, 500, Data);
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

				char buf[500];
				ifstream is(FilePath);
				is.getline(buf, 500);
				is.close();
				remove(FilePath);

				wchar_t *wBuf = UTF8ToUnicode(buf);
				char *AnsiBuf = UnicodeToANSI(wBuf);
				strcpy_s(item.Data, AnsiBuf);

				files.push(item);

				MySQL_ChangeStatus(item.RunID, ProgramStateStr[Queuing]);
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
	printf("%s �� ��ʼ����\n", MySQL_GetOJName());

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

			//����
			char Problem[100];
			//�û���
			char UserName[100];
			//����
			char Lang[100];
			//���Ե�
			char Test[500];
			//���ʱ��
			int MaxTime;
			//����ڴ�
			int MaxMemory;
			FileToData(coData.top().Data, Problem, MaxTime, MaxMemory, UserName, Lang, Test);


			char Path1[100], Path2[100];
			if (!strcmp(Lang, "Java"))
			{
				sprintf_s(Path1, "Temporary_Code\\%d", RunID);
				sprintf_s(Path2, "test\\%d\\Main.java", RunID);
			}
			else if (!strcmp(Lang, "Python"))
			{
				sprintf_s(Path1, "Temporary_Code\\%d", RunID);
				sprintf_s(Path2, "test\\%d\\Code.py", RunID);
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
