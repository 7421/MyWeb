#include "AsyncLog.h"

#include <ctime>
#include <time.h>
#include <sys/timeb.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <stdarg.h>

#include <unistd.h>

#define DEFAULT_ROLL_SIZE 10 * 1024 * 1024

FILE* CAsyncLog::m_hLogFile = NULL;
std::string CAsyncLog::m_strFileName = "default";
std::string CAsyncLog::m_strFileNamePID = "";
int64_t CAsyncLog::m_nFileRollSize = DEFAULT_ROLL_SIZE;
int64_t CAsyncLog::m_nCurrentWrittenSize = 0;
std::list<std::string> CAsyncLog::m_listLinesToWrite;
std::unique_ptr<std::thread> CAsyncLog::m_spWriteThread;
std::mutex	CAsyncLog::m_mutexWrite;
std::condition_variable CAsyncLog::m_cvWrite;
bool CAsyncLog::CAsyncLog::m_bExit = false;

void CAsyncLog::init(const char* pszLogFileName /* = nullptr*/, int64_t nRollSize /* 10 * 1024 * 1024 */)
{
	m_nFileRollSize = nRollSize;
	if (pszLogFileName == nullptr || pszLogFileName[0] == 0)
	{
		m_strFileName.clear();
	}
	else
		m_strFileName = pszLogFileName;

	//��ȡ����id���������ٿ���ͬһ�����̵Ĳ�ͬ��־�ļ�
	char szPID[8];
	snprintf(szPID, sizeof(szPID), "%05d", (int)::getpid());
	m_strFileNamePID = szPID;

	m_spWriteThread.reset(new std::thread(writeThreadProc));
}

void CAsyncLog::uninit()
{
	while (true)
	{
		{
			std::unique_lock<std::mutex> guard(m_mutexWrite,std::try_to_lock);
			if (guard.owns_lock())
			{
				if (m_listLinesToWrite.empty())
					break;
			}
		}
	}
	m_bExit = true;
	m_cvWrite.notify_one();
	//���ж�һ���߳��Ƿ��Ѿ��˳����ٵȴ��߳̽���
	if (m_spWriteThread->joinable())
		m_spWriteThread->join();

	if (m_hLogFile != nullptr)
	{
		fclose(m_hLogFile);
		m_hLogFile = nullptr;
	}
}

void CAsyncLog::output(long nLevel, const char* pszFileName, int nLineNo, const char* pszFmt, ...)
{
	std::string strLine;
	makeLinePrefix(nLevel, strLine);
	//����ǩ��
	char szFileName[512] = { 0 };
	snprintf(szFileName, sizeof(szFileName), "[%s:%d]", pszFileName, nLineNo);
	strLine += szFileName;

	//Log ����
	std::string strLogMsg;
	//�ȼ���һ�²��������ĳ��ȣ��Ա��ڷ���ռ�
	va_list ap;
	va_start(ap, pszFmt);
	int nLogMsgLength = vsnprintf(NULL, 0, pszFmt, ap);
	va_end(ap);

	strLogMsg.resize(nLogMsgLength + 1);

	va_list aq;
	va_start(aq, pszFmt);
	vsnprintf((char*)strLogMsg.data(), strLogMsg.capacity(), pszFmt, aq);
	va_end(aq);


	strLine += " : ";
	strLine += strLogMsg;
	//�������������̨�Ż���ÿһ��ĩβ��һ�����з�
	if (!m_strFileName.empty())
	{
		strLine += "\n";
	}
	{
		std::lock_guard<std::mutex> lock_guard(m_mutexWrite);
		m_listLinesToWrite.push_back(strLine);
	}
	m_cvWrite.notify_one();

	if(nLevel >= LOG_LEVEL_ERROR)
	{
		uninit();
		crash();
	}
}

void CAsyncLog::makeLinePrefix(long nLevel, std::string& strPrefix)
{
	// ����
	strPrefix = "[INFO]";
	if (nLevel == LOG_LEVEL_WARNING)
		strPrefix = "[WARN]";
	else if (nLevel == LOG_LEVEL_ERROR)
		strPrefix = "[ERROR]";
	
	//ʱ��
	char szTime[64] = { 0 };
	getTime(szTime, sizeof(szTime));
	strPrefix += "[";
	strPrefix += szTime;
	strPrefix += "]";
	//��ǰ�߳���Ϣ
	char szThreadID[32] = { 0 };
	std::ostringstream osThreadID;
	osThreadID << std::this_thread::get_id();
	snprintf(szThreadID, sizeof(szThreadID), "[%s]", osThreadID.str().c_str());
	strPrefix += szThreadID;
}

void CAsyncLog::getTime(char* pszTime, int nTimeStrLength)
{
	struct timeb tp;
	ftime(&tp);

	time_t now = tp.time;
	tm time;

	localtime_r(&now, &time);
	snprintf(pszTime, nTimeStrLength, "[%04d-%02d-%02d %02d:%02d:%02d:%03d]", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday, time.tm_hour, time.tm_min, time.tm_sec, tp.millitm);
}

bool CAsyncLog::createNewFile(const char* pszLogFileName)
{
	if (m_hLogFile != nullptr)
	{
		fclose(m_hLogFile);
	}
	//ʼ���½��ļ�
	m_hLogFile = fopen(pszLogFileName, "w+");
	return m_hLogFile != nullptr;
}

bool CAsyncLog::writeToFile(const std::string& data)
{
	//Ϊ�˷�ֹ���ļ�һ����д���꣬����һ��ѭ���������д
	std::string strLocal(data);
	int ret = 0;
	while (true)
	{
		ret = fwrite(strLocal.c_str(), 1, strLocal.length(), m_hLogFile);
		if (ret < 0)
			return false;
		else if (ret <= (int)strLocal.length())
		{
			strLocal.erase(0, ret);
		}

		if (strLocal.empty())
			break;
	}
	//ǿ�ƻ������ڵ����ݣ����뵽�ļ�����
	fflush(m_hLogFile);
	return true;
}


void CAsyncLog::crash()
{
	char* p = nullptr;
	*p = 0;
}

void CAsyncLog::writeThreadProc()
{
	while (true)
	{
		if (!m_strFileName.empty())
		{
			if (m_hLogFile == nullptr || m_nCurrentWrittenSize >= m_nFileRollSize)
			{
				//����m_nCurrentWrittenSize��С
				m_nCurrentWrittenSize = 0;

				//��һ�λ����ļ���С����rollsize�����½��ļ�
				char szNow[64];
				time_t now = time(NULL);
				tm time;

				localtime_r(&now, &time);
				strftime(szNow, sizeof(szNow), "%Y%m%d%H%M%S", &time);

				std::string strNewFileName(m_strFileName);
				strNewFileName += '.';
				strNewFileName += szNow;
				strNewFileName += ".";
				strNewFileName += m_strFileNamePID;
				strNewFileName += ".log";
				if (!createNewFile(strNewFileName.c_str()))
					return;
			}
		}

		std::string strLine;
		{
			std::unique_lock<std::mutex> guard(m_mutexWrite);
			while (m_listLinesToWrite.empty())
			{
				if (m_bExit)
					return;
				m_cvWrite.wait(guard);
			}

			strLine = m_listLinesToWrite.front();
			m_listLinesToWrite.pop_front();
		}
		std::cout << strLine << std::endl;

		if (!m_strFileName.empty())
		{
			if (!writeToFile(strLine))
				return;

			m_nCurrentWrittenSize += strLine.length();
		}
	}
}