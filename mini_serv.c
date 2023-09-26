#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>

int		ft_strlen(const char *str);
void	panic(const char *msg);
int		integrateNewConnection(int max_fds, int masterSocket, fd_set *fds, struct sockaddr *addr, socklen_t *len);
void	broadcast(int max_fds, int socketMaster, char *msg, fd_set *fds);

int	clients[9999] = { -1 };
int	id = 1;

int	main(int c, char **v)
{
	int					res;
	int 				sMaster;
	int					max_fd;
	socklen_t			len;
	char				buffer[999999];
	struct sockaddr_in	addr;
	fd_set				set;
	fd_set				w_set;
	fd_set				r_set;

	if (c != 2)
		panic("Wrong number of arguments\n");
	sMaster = socket(AF_INET, SOCK_STREAM, 0);
	if (sMaster < 0)
		panic("Fatal error\n");
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_port = htons(atoi(v[1]));
	len = sizeof(struct sockaddr_in);
	res = bind(sMaster, (struct sockaddr *) &addr, len);
	if (res < 0)
		panic("Fatal error\n");
	res = listen(sMaster, 128);
	if (res < 0)
		panic("Fatal error\n");
	FD_ZERO(&set);
	FD_ZERO(&w_set);
	FD_ZERO(&r_set);
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
			if (!FD_ISSET(i, &r_set))
				continue ;
			bzero(buffer, sizeof(buffer));
			printf("testing 1\n");
			if (i == sMaster)
			{
				max_fd = integrateNewConnection(max_fd, sMaster, &set, (struct sockaddr *) &addr, &len);
				sprintf(buffer, "server: client %d just arrived\n", id);
				broadcast(max_fd, sMaster, buffer, &set);
			}
			else
			{
				res = recv(i, buffer, sizeof(buffer), 0);
				if (res < 0)
					panic("Fatal error\n");
				if (res == 0)
				{
					FD_CLR(i, &set);
					close(i);
					sprintf(buffer, "server: client %d just left\n", clients[i]);
					broadcast(max_fd, sMaster, buffer, &w_set);
					clients[i] = -1;
					continue ;
				}
				broadcast(max_fd, sMaster, buffer, &w_set);
			}
		}
	}
	return (0);
}

void	panic(const char *msg)
{
	write(2, msg, ft_strlen(msg));
	exit(EXIT_FAILURE);
}

int		ft_strlen(const char *str)
{
	int	i;

	i = -1;
	if (str)
		while (str[++i]);
	return (i);
}

int		integrateNewConnection(int max_fds, int masterSocket, fd_set *fds, struct sockaddr *addr, socklen_t *len)
{
	int	res;

	res = accept(masterSocket, addr, len);
	if (res < 0)
		panic("Fatal error\n");
	FD_SET(res, fds);
	clients[res] = id++;
	res = res > max_fds ? res : max_fds;
	return (res);
}

void	broadcast(int max_fds, int socketMaster, char *msg, fd_set *fds)
{
	for (int i = 0; i <= max_fds; i++)
	{
		if (FD_ISSET(i, fds))
			if (i != socketMaster)
				send(i, msg, ft_strlen(msg), 0);
	}
}
