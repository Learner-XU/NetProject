/*
  This example program provides a trivial server program that listens for TCP
  connections on port 9995.  When they arrive, it writes a short message to
  each client connection, and closes each connection once it is flushed.

  Where possible, it exits cleanly in response to a SIGINT (ctrl-c).
*/


#include "Reactor.h"
#include "Timer.h"
#include "SignalManager.h"
int main(int argc, char** argv)
{
	XLog::GetInst().Init("NetProject");

	LOG(INFO)("NetProject Program start!");
	XReactor reactor;
	Timer timer;
	SignalManager signalManager;
	reactor.connectCreate(1994);
	timer.Create();
	timer.run();
	signalManager.Create();
	signalManager.run();
	reactor.Loop();
	
	while (1) 
	{
		sleep(3);
	}
	reactor.connectClose(NULL);
	signalManager.Close();
	timer.Close();
	LOG(INFO)("NetProject Program stop!");
	XLog::GetInst().Destory();
	return 0;
}

