#pragma once

#define PathLen 200
#define CompilerPath "D:\\Program Files (x86)\\CodeBlocks\\MinGW\\bin"
#define JavaPath ""

enum ProgramState
{
	Wating,
	Queuing,
	Compiling,
	Running,
	Accepted,
	PresentationError,
	TimeLimitExceeded,
	MemoryLimitExceeded,
	WrongAnswer,
	RuntimeError,
	OutputLimitExceeded,
	CompileError,
	SystemError
};

const char ProgramStateStr[][30]
{
	"Wating",
	"Queuing",
	"Compiling",
	"Running",
	"Accepted",
	"Presentation Error",
	"Time Limit Exceeded",
	"Memory Limit Exceeded",
	"Wrong Answer",
	"Runtime Error",
	"Output Limit Exceeded",
	"Compile Error",
	"System Error"
};

class JudgeSystem_t
{
public:

private:
	//��ʱ״̬����ÿ�����Ե������״̬
	int status;
	//��״̬
	int JudgeStatus;
	//���Ա�����
	char LanguageType[10];
	//���볤��
	long CodeLen;
	//���ʱ������
	int MaxTime;
	//����ڴ�����
	int MaxMemory;

	bool HaveCompile;
private:
	bool CheckCode_C(int RunID);
	bool CheckCode_Java(int RunID);
	bool CheckCode_Python(int RunID);
	bool AcceptedTest(const char* program, const char* tester);
	bool PresentationErrorTest(const char* program, const char* tester);
	bool OutputLimitExceededTest(const char* program, const char* tester);
	//����c++
	bool Compiler_C(int RunID);
	bool Compiler_JAVA(int RunID);
	void Judge(int RunID, const char *Problem, int TestID, DWORD &exitcode, int &Time, int &Memory);
public:
	JudgeSystem_t();
	~JudgeSystem_t();

	long GetCodeLen();
	//���ñ�������
	void SetLanguage(const char *cmp);
	//��ȡ��������
	char* GettLanguage();
	//�����ļ���
	void CreateDir(int RunID);
	//��ȡ״̬��typeΪfalseʱ��ȡ���Ե��״̬������Ϊ��״̬
	int GetStatus(bool type);
	//�������������һ�лָ�Ϊ��ʼ״̬
	void Remark();
	//�������⣬������
	void ContinueJudge();
	//ɾ����������
	void DeleteDir(int RunID);
	//��ȡ���������Ϣ
	char* GetErrorData(int RunID);
	//�������ʱ������
	void SetLimitTime(int MaxTime);
	//��������ڴ�����
	void SetLimitMemory(int MaxMemory);
	//����
	int StartJudge(int RunID, const char *Problem, int TestID, DWORD &exitcode, int &Time, int &Memory);
};


