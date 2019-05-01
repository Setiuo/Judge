#include "JudgerType.h"

#include <stdio.h>
#include <iostream>
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "MySql.h"
using namespace std;
using namespace sql;

Driver *driver;
Connection *con;
Statement *stmt;
ResultSet *res;

extern void MySQL_Connect()
{
	try
	{
		driver = get_driver_instance();
		con = driver->connect("tcp://127.0.0.1", "root", "SqlPassWord");
		con->setSchema("openjudge");

		stmt = con->createStatement();
	}
	catch (sql::SQLException &e)
	{
		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}
	}
}

extern int MySQL_GetOJAllRun()
{
	int AllRun = 0;

	try
	{
		res = stmt->executeQuery("SELECT * FROM oj_data");
		res->next();
#ifndef JUDGE_CONTEST
		AllRun = res->getInt(7);
#endif
#ifdef JUDGE_CONTEST
		AllRun = res->getInt(8);
#endif
	}
	catch (sql::SQLException &e)
	{

		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}

	}

	return AllRun;
}
extern void MySQL_SetOJAllRun()
{
	try
	{
		static char cmd[100];
#ifndef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_data set oj_allrun_1=%d where oj_name=\"%s\"", MySQL_GetOJAllRun() + 1, MySQL_GetOJName());
#endif
#ifdef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_data set oj_allrun_2=%d where oj_name=\"%s\"", MySQL_GetOJAllRun() + 1, MySQL_GetOJName());
#endif
		res = stmt->executeQuery(cmd);
	}
	catch (sql::SQLException &e)
	{

		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}

	}
}

extern void MySQL_SetOJState(int State)
{
	try
	{
		static char cmd[100];
#ifndef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_data set oj_EvaMacState_1=%d where oj_name=\"%s\"", State, MySQL_GetOJName());
#endif
#ifdef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_data set oj_EvaMacState_2=%d where oj_name=\"%s\"", State, MySQL_GetOJName());
#endif
		res = stmt->executeQuery(cmd);
	}
	catch (sql::SQLException &e)
	{

		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}

	}
}

extern void MySQL_SetJudgerName(int RunID, const char *Name)
{
	try
	{
		static char cmd[100];
#ifndef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_status set Judger=\"%s\" where RunID=%d", Name, RunID);
#endif
#ifdef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_constatus set Judger=\"%s\" where RunID=%d", Name, RunID);
#endif
		res = stmt->executeQuery(cmd);
	}
	catch (sql::SQLException &e)
	{
		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}
	}
}

extern void MySQL_ChangeStatus(int RunID, const char *Status)
{
	try
	{
		static char cmd[100];
#ifndef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_status set status=\"%s\" where RunID=%d", Status, RunID);
#endif
#ifdef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_constatus set status=\"%s\" where RunID=%d", Status, RunID);
#endif
		res = stmt->executeQuery(cmd);
	}
	catch (sql::SQLException &e)
	{
		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}
	}
}

extern void MySQL_SetAllStatus(int RunID, const char *Status)
{
	try
	{
		static char cmd[1000];
#ifndef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_status set AllStatus=\"%s\" where RunID=%d", Status, RunID);
#endif
#ifdef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_constatus set AllStatus=\"%s\" where RunID=%d", Status, RunID);
#endif
		res = stmt->executeQuery(cmd);
	}
	catch (sql::SQLException &e)
	{
		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}
	}
}

extern void MySQL_SetUseMemory(int RunID, int UseMemory)
{
	try
	{
		static char cmd[100];
#ifndef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_status set UseMemory=%d where RunID=%d", UseMemory, RunID);
#endif
#ifdef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_constatus set UseMemory=%d where RunID=%d", UseMemory, RunID);
#endif
		res = stmt->executeQuery(cmd);
	}
	catch (sql::SQLException &e)
	{

		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}

	}
}

extern void MySQL_SetUseTime(int RunID, int UseTime)
{
	try
	{
		static char cmd[100];
#ifndef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_status set UseTime=%d where RunID=%d", UseTime, RunID);
#endif
#ifdef JUDGE_CONTEST
		sprintf_s(cmd, "update oj_constatus set UseTime=%d where RunID=%d", UseTime, RunID);
#endif
		res = stmt->executeQuery(cmd);
	}
	catch (sql::SQLException &e)
	{

		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}
		
	}
}

extern char *MySQL_GetOJName()
{
	static char Name[20];
	try
	{
		res = stmt->executeQuery("SELECT * FROM oj_data");
		res->next();
		strcpy_s(Name, res->getString(1).c_str());

	}
	catch (sql::SQLException &e)
	{

		if (e.getErrorCode() != 0)
		{
			cout << "SQLException in " << __FILE__;
			cout << " (" << __FUNCTION__ << ") on line " << __LINE__ << endl;
			cout << "" << e.what();
			cout << " (MySQL error code: " << e.getErrorCode();
			cout << " , SQLState: " << e.getSQLState() << " )" << endl;
		}

	}

	return Name;
}

extern void MySQL_Delete()
{
	delete res;
	delete stmt;
	delete con;
}
