/*
  This example program provides a trivial server program that listens for TCP
  connections on port 9995.  When they arrive, it writes a short message to
  each client connection, and closes each connection once it is flushed.

  Where possible, it exits cleanly in response to a SIGINT (ctrl-c).
*/


#include "Reactor.h"


int main(int argc, char** argv)
{

	XReactor reactor;
	reactor.Init();
	reactor.connectCreate(1994);

	reactor.connectClose(NULL);
	return 0;
}

