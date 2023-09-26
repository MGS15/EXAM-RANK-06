#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

void	panic(const char *msg);
size_t	ft_strlen(const char *str);
void	new_connection(int master, struct sockaddr *addr, socklen_t *len);
void	broadcast(int master, int max, int exclude);
void	disconnect(int master, int index);

char	send_buffer[99999];
char	read_buffer[99999];
fd_set	set;
fd_set	r_set;
fd_set	w_set;
int		clients[99999] = { -1 };
int		id;
int		max_fd;

int	main(int c, char **v)
{
	int					res;
	int					sMaster;
	socklen_t			len;
	struct sockaddr_in	addr;


	if (c != 2)
		panic("Wrong number of arguments\n");
	sMaster = socket(AF_INET, SOCK_STREAM, 0);
	if (sMaster < 0)
		panic("Fatal error\n");
	bzero(&addr, sizeof(addr));
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons(atoi(v[1]));
	addr.sin_family = AF_INET;
	len = sizeof(addr);
	res = bind(sMaster, (struct sockaddr *) &addr, len);
	if (res < 0)
		panic("Fatal error\n");
	res = listen(sMaster, 128);
	if (res < 0)
		panic("Fatal error\n");
	FD_ZERO(&set);
	FD_ZERO(&r_set);
	FD_ZERO(&w_set);
	FD_SET(sMaster, &set);
	max_fd = sMaster;
	while (1)
	{
		w_set = set;
		r_set = set;
		res = select(max_fd + 1, &r_set, &w_set, NULL, NULL);
		if (res < 0)
			panic("Fatal error\n");
		for (int i = 0; i <= max_fd; i++)
		{
			bzero(read_buffer, sizeof(read_buffer));
			bzero(send_buffer, sizeof(read_buffer));
			if (!FD_ISSET(i, &r_set))
				continue ;
			if (i == sMaster)
				new_connection(sMaster, (struct sockaddr *) &addr, &len);
			else
			{
				res = recv(i, read_buffer, sizeof(read_buffer), 0);
				if (res == 0)
					disconnect(sMaster, i);
				else if (res > 0)
				{
					sprintf(send_buffer, "client %d: %s", clients[i], read_buffer);
					broadcast(sMaster, max_fd, i);
				}
			}
		}
	}
}

void	panic(const char *msg)
{
	write(2, msg, ft_strlen(msg));
	exit(EXIT_FAILURE);
}

size_t	ft_strlen(const char *str)
{
	int	i;

	i = -1;
	if (str)
		while (str[++i]);
	return (i);
}

void	broadcast(int master, int max, int exclude)
{
	for (int i = 0; i <= max; i++)
	{
		if (FD_ISSET(i, &set))
		{
			if (i != master && i != exclude)
				send(i, send_buffer, ft_strlen(send_buffer), 0);
		}
	}
}

void	new_connection(int master, struct sockaddr *addr, socklen_t *len)
{
	int	res;

	res = accept(master, addr, len);
	if (res < 0)
		panic("Fatal error\n");
	clients[res] = id++;
	FD_SET(res, &set);
	max_fd = res > max_fd ? res : max_fd;
	sprintf(send_buffer, "server: client %d just arrived\n", clients[res]);
	broadcast(master, max_fd, res);
}

void	disconnect(int master, int index)
{
	FD_CLR(index, &set);
	close(index);
	sprintf(send_buffer, "server: client %d just left\n", clients[index]);
	broadcast(master, max_fd, index);
}
