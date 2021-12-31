#include "Client.hpp"

Client::Client(const int& sock, const std::string& id) : socket_client(sock), id(id)
{
	size_body_mysql = -1;
	size_body_client = -1;
	is_write_to_mysql = false;
	is_write_to_client = false;
	is_close = false;
	file_log = nullptr;

	struct sockaddr_in  addr;
	socket_mysql = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_mysql != -1)
	{
		addr.sin_family = AF_INET;
		addr.sin_port = htons(3306);
		addr.sin_addr.s_addr = inet_addr("127.0.0.1");
		if (connect(socket_mysql, (struct sockaddr *) &addr, sizeof(addr)) != -1)
			fcntl(socket_mysql, F_SETFL, O_NONBLOCK);
		else
		{
			std::cerr << "errot connect" << std::endl;
			close(socket_mysql);
			socket_mysql = -1;
		}
	}
	else
		std::cerr << "error socket mysql\n";
}

Client::~Client() {}

Client::Client(const Client& oth)
{ *this = oth; }

Client&		Client::operator= (const Client& oth)
{
	this->socket_client = oth.socket_client;
	this->socket_mysql = oth.socket_mysql;
	this->packet_mysql = oth.packet_mysql;
	this->packet_client = oth.packet_client;
	this->is_write_to_mysql = oth.is_write_to_mysql;
	this->is_write_to_client = oth.is_write_to_client;
	this->is_close = oth.is_close;
	this->size_body_mysql = oth.size_body_mysql;
	this->size_body_client = oth.size_body_client;
	this->id = oth.id;
	this->file_log = oth.file_log;
	return (*this);
}

const int&		Client::get_socket_client() const
{ return (socket_client); }

const int&		Client::get_socket_mysql() const
{ return (socket_mysql); }

const bool&		Client::get_is_write_to_mysql() const
{ return (is_write_to_mysql); }

const bool&		Client::get_is_write_to_client() const
{ return (is_write_to_client); }

const bool&		Client::get_is_close() const
{ return (is_close); }

void			Client::set_file_log(FILE* file)
{ file_log = file; }

int				Client::write_socket_mysql()
{
	int bytes = write(socket_mysql, packet_client.c_str(), packet_client.size());
	if (bytes > 0)
	{
		packet_client.erase(0, bytes);
		if (packet_client.empty())
			is_write_to_mysql = false;
	}
	return (bytes);
}

int				Client::read_socket_mysql(const char *buf, const int& bytes)
{
	if (bytes < 0)
		return (bytes);
	packet_mysql.append(buf, bytes);
	is_write_to_client = true;
	return (bytes);
}

int				Client::write_socket_client()
{
	int bytes = write(socket_client, packet_mysql.c_str(), packet_mysql.size());
	if (bytes > 0)
	{
		packet_mysql.erase(0, bytes);
		if (packet_mysql.empty())
			is_write_to_client = false;
	}
	return (bytes);
}

int				Client::read_socket_client(const char* buf, int bytes)
{
	if (bytes < 0)
		return (bytes);

	packet_client.append(buf, bytes);

	if (size_body_client == -1 && packet_client.size() >= HEADER_BYTE)
	{
		size_body_client = (unsigned char)packet_client.c_str()[0]
			+ ((unsigned char)packet_client.c_str()[1] << 8)
			+ ((unsigned char)packet_client.c_str()[2] << 16)
			+ HEADER_BYTE;
	}

	if (packet_client.size() == (size_t)size_body_client)
	{
		if (3 == (unsigned char)packet_client.c_str()[4])
			create_log(packet_client.c_str() + 7);
		if (1 == (unsigned char)packet_client.c_str()[4])
			is_close = true;
		size_body_client = -1;
		is_write_to_mysql = true;
	}
	return (bytes);
}

void			Client::create_log(const char* s)
{
	std::string		log;

	log.assign(get_new_time());
	log.append(" | ");
	log.append(id);
	log.append(" | ");
	log.append(s);
	log.append("\n");

	fputs(log.c_str(), file_log);
	fflush(file_log);
}

std::string		Client::get_new_time()
{
	struct timeval	tv;
	struct tm		*info;
	char			buffer[64];
 
	gettimeofday(&tv, nullptr);
	info = localtime(&tv.tv_sec);
	strftime (buffer, sizeof buffer, "%x %X %Y", info);

	return (buffer);
}
