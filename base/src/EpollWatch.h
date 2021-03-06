/*
 * EpollWatch.h
 *
 *  Created on: 2015年9月1日
 *      Author: dengxu
 */

#ifndef BASE_SRC_EPOLLWATCH_H_
#define BASE_SRC_EPOLLWATCH_H_

#include<sys/epoll.h>
#include<signal.h>
#include<unistd.h>
#include<sys/eventfd.h>
#include"Mutex.h"

#include<vector>
using namespace std;

namespace lynetx {

class CEpollWatch
{
public:
	CEpollWatch();
	virtual ~CEpollWatch();
public:
	struct EpollWaitOjbect
	{
		int epollfd;    					//epoll对应的fd
		epoll_event event;					//epoll事件通知
		vector<epoll_event> vEvents;		//监听文件描述符集
		size_t eventSize;					//vEvents数组的大小
		size_t maxNumbers;					//当前最大文件描述符个数
		__sigset_t siget;					//信号事件句柄
	};
protected:
	inline void SafeChangeEvents()
	{
		if(m_object.maxNumbers > m_object.eventSize)
		{
			SafeIncreaseEvent();
		}
		else if(m_object.maxNumbers+1000<m_object.eventSize)
		{
			SafeDecreaseEvent();
		}
	}
	inline void SafeIncreaseEvent()
	{
		AUTO_GUARD(g, THREAD_MUTEX, m_mutex);
		if(m_object.maxNumbers > m_object.eventSize)
		{
			m_object.eventSize += 1000;
			m_object.vEvents.resize(m_object.eventSize);
		}
	}
	inline void SafeDecreaseEvent()
	{
		AUTO_GUARD(g, THREAD_MUTEX, m_mutex);
		if(m_object.maxNumbers+1000<m_object.eventSize)
		{
			m_object.eventSize -= 1000;
			m_object.vEvents.resize(m_object.eventSize);
		}
	}
	inline int SetEpollSignalObject(int signo)
	{
		sigemptyset(&m_object.siget);
		return sigaddset(&m_object.siget, signo);
	}
public:
	inline void DisableEpollWait()
	{
		this->m_bEpollWait=false;
	}
public:
	int EpollCreate();
	int EpollCtl(const int eopt, const int fd, const int events);
	int EpollAdd(const int fd, const int events);
	int EpollModify(const int fd, const int events);
	int EpollDel(const int fd, const int events);
	virtual int IoEpollWait(int timeout);
	int EnableWakeup();
	void DisableWakeup();
	void PushWakeup2Epoll(const int events);
	void EpollWakeup();
	void EpollHandleRead();

protected:
	int 		m_wakeupfd;				//唤醒句柄
	CMutex		m_mutex;					//自动线程同步变量
	bool		m_enableWakeup;			//判断是否使用了唤醒
	bool		m_bEpollWait;			//ture表示循环调用IoEpollWait函数
	struct EpollWaitOjbect m_object;//epoll操作句柄
};

} /* namespace lynetx */

#endif /* BASE_SRC_EPOLLWATCH_H_ */
