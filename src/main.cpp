#include "Server.hpp"

int	main()
{
	try
	{
		Server			server;

		if (server.createSListen("127.0.0.1", "9999") == -1)
			return (1);
		if (server.start() == -1)
			return (1);
		return (0);
	}
	catch (std::bad_alloc &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return (1);
}
