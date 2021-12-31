#ifndef CLIENT_HPP
# define CLIENT_HPP

# define HEADER_BYTE 4
# define BUF_SIZE 16777216

# include <iostream>
# include <fcntl.h>
# include <sys/time.h>
# include <arpa/inet.h> //sockaddr_in
# include <cstring>
# include <unistd.h>

class	Client
{
	public:
		Client(const int& sock, const std::string& id);
		Client(const Client& oth);
		~Client();

		Client&			operator= (const Client& oth);
		const int&		get_socket_client() const;
		const int&		get_socket_mysql() const;
		const bool&		get_is_write_to_mysql() const;
		const bool&		get_is_write_to_client() const;
		const bool&		get_is_close() const;
		void			set_file_log(FILE* file);
		int				write_socket_mysql();
		int				read_socket_mysql(const char *buf, const int& bytes);
		int				write_socket_client();
		int				read_socket_client(const char* buf, int bytes);
		void			create_log(const char* s);
		std::string		get_new_time();

	private:
		int				socket_client;
		int				socket_mysql;
		std::string		packet_mysql;
		std::string		packet_client;
		bool			is_write_to_mysql;
		bool			is_write_to_client;
		bool			is_close;
		int				size_body_mysql;
		int				size_body_client;
		std::string 	id;
		FILE*			file_log;
};

#endif
