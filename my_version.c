#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct s_client {
	int fd;
	int id;
}				t_client;

t_client	client[1024];
fd_set r_s, w_s, mem_s;
char	buff[1000000];
char	msg[1000014];

int	id;

void	fatal()
{
	write(1, "Fatal error\n", 12);
	exit(1);
}


void	send_to_all_clients(int from_fd)
{
	int i = 0;
	while (i < 1024)
	{
		if (client[i].id >= 0 && client[i].fd != from_fd && FD_ISSET(client[i].fd, &w_s))
		{
			if (send(client[i].fd, msg, strlen(msg), 0) < 0)
			{
				fatal();
			}
		}
		i++;
	}
	bzero(msg, sizeof(msg));
}

void	add_client(int clientfd)
{
	int i = 0;
	while (client[i].fd > 0 && i < 1024)
		i++;
	client[i].id = id++;
	client[i].fd = clientfd;
	FD_SET(clientfd, &mem_s);
	sprintf(msg, "server: client %d just arrived\n", client[i].id);
	write(1, "new client arrived\n", 19);
	send_to_all_clients(clientfd);
}


void	send_message(int i)
{
	sprintf(msg, "client %d: ", client[i].id);
	int len_intro = strlen(msg);
	size_t j = 0;
	while (j < strlen(buff))
	{
		msg[len_intro + j] = buff[j];
		j++;
	}

	// printf("The len of msg is : %ld\n", strlen(msg));
	send_to_all_clients(client[i].fd);
}

int main(int argc, char const *argv[])
{
	int sockfd, fd_client, fd_max;
	struct sockaddr_in servaddr, cli;
	socklen_t len;

	if (argc < 2)
	{
		printf("Wrong number of arguments");
		return 1;
	}
	else
	{
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = htonl(2130706433);
		servaddr.sin_port = htons(atoi(argv[1]));
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0 || bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0 || listen(sockfd, 10) < 0)
			fatal();
		fd_max = sockfd;
		FD_SET(sockfd, &mem_s);
		while (1)
		{
			r_s = w_s = mem_s;
			if (select(fd_max+ 1, &r_s, &w_s, NULL, NULL) < 0)
				fatal();
			if (FD_ISSET(sockfd, &r_s))
			{
				len = sizeof(cli);
				fd_client = accept(sockfd, (struct sockaddr *)&cli, &len);
					if (fd_client < 0)
						fatal();
				if (fd_client > fd_max)
					fd_max = fd_client;
				add_client(fd_client);
			}
			for (int i = 0; i < 1024; i++)
			{
				if (client[i].id < 0 || !FD_ISSET(client[i].fd, &r_s))
					continue;
				bzero(buff, sizeof(buff));
				int rval = 1;
				while (rval)
				{
					rval = recv(client[i].fd, buff+strlen(buff),1,0);
					if (buff[strlen(buff) - 1] == '\n')
						break;
				}
				if (!rval)
				{
					sprintf(msg, "server: client %d just left\n", client[i].id);
					send_to_all_clients(client[i].fd);
					FD_CLR(client[i].fd, &mem_s);
					close(client[i].fd);
					client[i].fd = client[i].id = -1;
				}
				else
					send_message(i);
			}
		}
	}
	return 0;
}
