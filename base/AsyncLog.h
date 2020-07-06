#pragma once

#include <stdio.h>
#include <string>
#include <list>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>

enum LOG_LEVEL
{
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR		//用于业务错误
};


#define LOGI(...) CAsyncLog::output(LOG_LEVEL_INFO,__FILE__,__LINE__,__VA_ARGS__)
#define LOGW(...) CAsyncLog::output(LOG_LEVEL_WARNING,__FILE__,__LINE__,__VA_ARGS__)
#define LOGE(...) CAsyncLog::output(LOG_LEVEL_ERROR,__FILE__,__LINE__,__VA_ARGS__)



class CAsyncLog
{
public:
	CAsyncLog() = delete;
	~CAsyncLog() = delete;

	CAsyncLog(const CAsyncLog& rhs) = delete;
	CAsyncLog& operator=(const CAsyncLog& rhs) = delete;
	//初始化日志线程
	static void init(const char* pszLogFileName = nullptr, int64_t nRollSize = 10 * 1024 * 1024);
	//关闭日志线程
	static void uninit();

	//输出线程ID号和所在函数签名、行号
	static void output(long nLevel, const char* pszFileName, int nLineNo, const char* pszFmt, ...);
private:
	//生成日志前缀 [级别] + [时间] + [当前线程]
	static void makeLinePrefix(long nLevel, std::string& sstrPrefix);
	static void getTime(char* pszTime, int nTimeStrLength);
	static bool createNewFile(const char* pszLogFileName);
	static bool writeToFile(const std::string& data);
	//让程序主动崩溃
	static void crash();

	static void writeThreadProc();

private:
	static bool								m_bToFile;				//日志写入文件还是写到控制台
	static FILE*							m_hLogFile;
	static std::string						m_strFileName;			//日志文件名
	static std::string						m_strFileNamePID;       //文件名中的进程ID
	static int64_t							m_nFileRollSize;        //单个日志文件的最大字节数
	static int64_t							m_nCurrentWrittenSize;  //已经写入的字节数目
	static std::list<std::string>			m_listLinesToWrite;		//待写入的日志
	static std::unique_ptr<std::thread>		m_spWriteThread;		//异步日志线程
	static std::mutex						m_mutexWrite;			//互斥锁
	static std::condition_variable			m_cvWrite;				//条件变量
	static bool								m_bExit;				//退出标志
};

