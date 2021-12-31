#ifndef SERVER_HPP
# define SERVER_HPP

# include <vector>
# include "Client.hpp"
# include "def_color.hpp"
# include <csignal>

class	Server
{
	public:
		Server();
		~Server();

		int			createSListen(const std::string& ip, const std::string& port);
		int			start();

	private:
		Server(const Server& oth);
		Server&					operator= (const Server& oth);
		void					addNewClient();
		int						print_error(const std::string& s);

		char					*buf;
		int						listen_socket;
		std::vector<Client>		list_client;
		bool					run;
		FILE*					file_log;
};

#endif
