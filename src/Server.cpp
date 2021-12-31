#include "Server.hpp"

Server::Server()
{
	buf = new char [BUF_SIZE];
	run = true;
	file_log = fopen("log.log", "a");
	if (!file_log)
		throw std::bad_alloc();
}

Server::~Server()
{
	fclose(file_log);
	delete [] buf;

	if (listen_socket != -1)
		close (listen_socket);

	std::vector<Client>::iterator	it_client = list_client.begin();
	for (; it_client != list_client.end(); ++it_client)
	{
		close((*it_client).get_socket_client());
		close((*it_client).get_socket_mysql());
	}
}

//CREATE_LISTEN_SOCKET_________________________________________________________
int		Server::createSListen(const std::string& ip, const std::string& port)
{
	//создание сокета__________________________________________________
	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == -1)
		return (print_error("socket"));

	//перевод сокета в неблокирующий режим_____________________________
	int	flags = fcntl(listen_socket, F_GETFL);
	if (-1 == fcntl(listen_socket, F_SETFL, flags | O_NONBLOCK))
		return (print_error("fcntl"));

	//установили флаг, для предотвращения залипаниz TCP порта__________
	int	opt = 1;
	if (-1 == setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
		return (print_error("setsockopt"));

	//сопостовление созданному сокету конкретного адреса_______________
	struct sockaddr_in  addr;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port.c_str()));
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
	if (-1 == bind(listen_socket,(struct sockaddr*)&addr, sizeof(addr)))
		return (print_error("bind"));

	//переводим сокет в слушаюший режим________________________________
	if (-1 == listen(listen_socket, 128))
		return (print_error("listen"));

	//печать на консоль информацию______________________________________
	std::cout << PINK << "  START PROXY SERVER ";
	std::cout << ip << ":" << port << "->127.0.0.1:3306" << NO_C << '\n';
	return (listen_socket);
}

int		Server::print_error(const std::string& s)
{
	std::cerr << "Error: " << s << ": " << strerror(errno) << '\n';
	return (-1);
}
//START_WEBSERVER______________________________________________________________
int		Server::start()
{
	fd_set							readfds;
	fd_set							writefds;
	int								max_d;
	int								bytes;
	std::vector<Client>::iterator	it_client;

	while (run)
	{
		max_d = 0;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);

		FD_SET(listen_socket, &readfds);
		max_d = std::max(listen_socket, max_d);

		for (it_client = list_client.begin(); it_client != list_client.end(); ++it_client)
		{
			FD_SET((*it_client).get_socket_client(), &readfds);
			FD_SET((*it_client).get_socket_mysql(), &readfds);

			if ((*it_client).get_is_write_to_client())
				FD_SET((*it_client).get_socket_client(), &writefds);
			if ((*it_client).get_is_write_to_mysql())
				FD_SET((*it_client).get_socket_mysql(), &writefds);

			max_d = std::max((*it_client).get_socket_client(), max_d);
			max_d = std::max((*it_client).get_socket_mysql(), max_d);
		}

		if (-1 == select(max_d + 1, &readfds, &writefds, NULL, NULL))
			return (0);

		if (FD_ISSET(listen_socket, &readfds))
			addNewClient();

		it_client = list_client.begin();
		while (it_client != list_client.end())
		{
			if (FD_ISSET((*it_client).get_socket_mysql(), &writefds))
				bytes = (*it_client).write_socket_mysql();
			else if (FD_ISSET((*it_client).get_socket_mysql(), &readfds))
				bytes = (*it_client).read_socket_mysql(buf,
					read((*it_client).get_socket_mysql(), buf, BUF_SIZE));

			if (bytes < 0 || (*it_client).get_is_close())
			{
				close((*it_client).get_socket_client());
				close((*it_client).get_socket_mysql());
				it_client = list_client.erase(it_client); continue;
			}

			if (FD_ISSET((*it_client).get_socket_client(), &writefds))
				bytes = (*it_client).write_socket_client();
			else if (FD_ISSET((*it_client).get_socket_client(), &readfds))
				bytes = (*it_client).read_socket_client(buf,
					read((*it_client).get_socket_client(), buf, BUF_SIZE));

			if (bytes < 0)
			{
				close((*it_client).get_socket_client());
				close((*it_client).get_socket_mysql());
				it_client = list_client.erase(it_client); continue;
			}

			++it_client;
		}
	}
	std::cout << "end while" << std::endl;
	return (0);
}

//ADD_NEW_CLIENT_______________________________________________________________
void	Server::addNewClient()
{
	int						socket_client;
	struct sockaddr_in		addr;
	socklen_t				aln;

	socket_client = accept(listen_socket, (struct sockaddr*) &addr, &aln);

	if (socket_client == -1)
		print_error("accept");
	else
	{
		std::string		id = inet_ntoa(addr.sin_addr);
		id.append(":");
		id.append(std::to_string(ntohs(addr.sin_port)));

		Client		client(socket_client, id);

		if (client.get_socket_mysql() == -1)
			close(socket_client);
		else
		{
			client.set_file_log(file_log);
			fcntl(socket_client, F_SETFL, O_NONBLOCK);
			list_client.push_back(client);
		}
	}
}
