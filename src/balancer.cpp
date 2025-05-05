#include "inetw.h"

#include "conn_engine.hpp"

#define PORT 8192


int main()
{
	ConnectionEngine conn_engine;

	conn_engine.setup("loadbalancer", INADDR_ANY, PORT);

	conn_engine.run();

	conn_engine.cleanup();

	return 0;
}
