#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <iostream>
#include <string>

#if defined(__APPLE__) || defined(__linux__)
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#endif


namespace xchip { namespace utility {
	
	
class Process
{
public:
	using ProcFunc = int(*)(void*);
	Process();
	~Process();
	Process(const Process&) = delete;
	const Process& operator=(const Process&) = delete;

	void Run(const std::string &app);
	void Run(ProcFunc pfunc, void* arg = nullptr);
	void Stop();
private:
#if defined(__APPLE__) || defined(__linux__)
	pid_t pid = 0;
#endif

};
	
	
	
	














}}









#endif
