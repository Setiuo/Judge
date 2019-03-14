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
	//临时状态，即每个测试点的运行状态
	int status;
	//总状态
	int JudgeStatus;
	//语言编译器
	char LanguageType[10];
	//代码长度
	long CodeLen;
	//最大时间限制
	int MaxTime;
	//最大内存限制
	int MaxMemory;

	bool HaveCompile;
private:
	bool CheckCode(int RunID);
	bool AcceptedTest(const char* program, const char* tester);
	bool PresentationErrorTest(const char* program, const char* tester);
	bool OutputLimitExceededTest(const char* program, const char* tester);
	//编译c++
	bool Compiler_C(int RunID);
	bool Compiler_JAVA(int RunID);
	void Judge(int RunID, const char *Problem, int TestID, DWORD &exitcode, int &Time, int &Memory);
public:
	JudgeSystem_t();
	~JudgeSystem_t();

	long GetCodeLen();
	//设置编译语言
	void SetLanguage(const char *cmp);
	//获取编译语言
	char* GettLanguage();
	//创建文件夹
	void CreateDir(int RunID);
	//获取状态，type为false时获取测试点的状态，否则为总状态
	int GetStatus(bool type);
	//重置评测机，即一切恢复为初始状态
	void Remark();
	//继续评测，不编译
	void ContinueJudge();
	//删除评测数据
	void DeleteDir(int RunID);
	//获取编译错误信息
	char* GetErrorData(int RunID);
	//设置最大时间限制
	void SetLimitTime(int MaxTime);
	//设置最大内存限制
	void SetLimitMemory(int MaxMemory);
	//评测
	int StartJudge(int RunID, const char *Problem, int TestID, DWORD &exitcode, int &Time, int &Memory);
};


