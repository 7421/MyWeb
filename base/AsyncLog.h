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
	LOG_LEVEL_ERROR		//����ҵ�����
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
	//��ʼ����־�߳�
	static void init(const char* pszLogFileName = nullptr, int64_t nRollSize = 10 * 1024 * 1024);
	//�ر���־�߳�
	static void uninit();

	//����߳�ID�ź����ں���ǩ�����к�
	static void output(long nLevel, const char* pszFileName, int nLineNo, const char* pszFmt, ...);
private:
	//������־ǰ׺ [����] + [ʱ��] + [��ǰ�߳�]
	static void makeLinePrefix(long nLevel, std::string& sstrPrefix);
	static void getTime(char* pszTime, int nTimeStrLength);
	static bool createNewFile(const char* pszLogFileName);
	static bool writeToFile(const std::string& data);
	//�ó�����������
	static void crash();

	static void writeThreadProc();

private:
	static bool								m_bToFile;				//��־д���ļ�����д������̨
	static FILE*							m_hLogFile;
	static std::string						m_strFileName;			//��־�ļ���
	static std::string						m_strFileNamePID;       //�ļ����еĽ���ID
	static int64_t							m_nFileRollSize;        //������־�ļ�������ֽ���
	static int64_t							m_nCurrentWrittenSize;  //�Ѿ�д����ֽ���Ŀ
	static std::list<std::string>			m_listLinesToWrite;		//��д�����־
	static std::unique_ptr<std::thread>		m_spWriteThread;		//�첽��־�߳�
	static std::mutex						m_mutexWrite;			//������
	static std::condition_variable			m_cvWrite;				//��������
	static bool								m_bExit;				//�˳���־
};

