#pragma once

//连接数据库
extern void MySQL_Connect();

//获取OJ名称
extern char *MySQL_GetOJName();

//设置评测机名称
extern void MySQL_SetJudgerName(int RunID, const char *Name);

//设置评测机运行状态
extern void MySQL_SetOJState(int State);

//设置评测机评测次数
extern void MySQL_SetOJAllRun();

//获取评测机评测次数
extern int MySQL_GetOJAllRun();

//设置数据库评测结果
extern void MySQL_ChangeStatus(int RunID, const int Status);

//设置数据库题的每个测试点的评测状态，包括时间内存等
extern void MySQL_SetAllStatus(int RunID, const char *Status);

//设置数据库评测结果： 使用的时间
extern void MySQL_SetUseTime(int RunID, int UseTime);

//设置数据库评测结果： 使用的内存
extern void MySQL_SetUseMemory(int RunID, int Memory);

//注销数据库
extern void MySQL_Delete();