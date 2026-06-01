#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h> //close()
#include<sys/socket.h> //It calls  socket(), connect()
#include<sys/types.h> //for SO_RCVTIMED AND SO_SNDTIMED
#include<netinet/in.h> // used in sockaddr_in , IPV4 socket addresses
#include<arpa/inet.h> // inet_addr()
#include<fcntl.h> // fcntl() for non-blocking
#include<errno.h>  // errno
#include <sys/time.h>

#define TIMEOUT_SEC 1 // seconds to wait per port



//Defining few port names for nicer output


const char *port_name(int port) { switch (port) {
		case 21: return "FTP";
	        case 22: return "SSH";
	        case 23: return "Telnet";
                case 25: return "SMTP";
                case 53: return "DNS";
	        case 80: return "HTTP";
	        case 110: return "POP3";
	        case 143: return "IMAP";
	        case 443: return "HTTPS";
                case 3306: return "MYSQL";
	        case 3389: return "RDP";
		case 5432: return "PostgresSQL";
	        default: return "unknown";

	}
}
/* scan_port() - tries to TCP-connect to ip:port, retruen 1 if open and 0 if closed/filtered */

int scan_port(const char *ip, int port){
	int sock;
	struct sockaddr_in target;
	struct timeval timeout;

	/* first we create a TCP socket */
	sock =socket(AF_INET,SOCK_STREAM,0);

	if (sock <0) return 0;

	/*second we creste a timeout so we dont wait forever*/

	timeout.tv_sec =TIMEOUT_SEC;
	timeout.tv_usec =0;
	setsockopt(sock, SOL_SOCKET,SO_RCVTIMEO,&timeout, sizeof(timeout));
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

	/*Thirdly we fill in the destination address*/
	memset(&target, 0, sizeof(target));
	target.sin_family =AF_INET;  //IPV4
	target.sin_port = htons(port); //port in network byte order
	target.sin_addr.s_addr = inet_addr(ip); //IP address
		//Lastly on the TCP Socket we try to connect- if it succeeds, the port is open
		int result = connect(sock, (struct sockaddr *)&target, sizeof(target));

	/* finally we close the socket after checking*/

	close(sock);

	return(result ==0); //0 means connected =open
}

	int main(){
		char ip[100];
		int start_port, end_port;
		int open_count= 0;

		//Input from user
		printf("\n == Mariquita Port Scanner == \n\n");
		printf("  Target IP:");
		fgets(ip, sizeof(ip), stdin);
		ip[strlen(ip) -1] = '\0';

		printf(" start port:");
		scanf("%d", &start_port);

		printf(" End Port: ");
		scanf("%d", &end_port);

		/* validating range */

		if (start_port < 1||end_port > 65535 || start_port > end_port) {
			printf("\n invalid port range, use 1-65535. \n\n");
			return 1;
		}

		printf("\nscanning%sports %d-%d ... \n", ip, start_port, end_port);
		printf(" %-8d%-10s%s\n", "PORT", "STATE", "SERVICE");


		/*Scanning each port*/
		for (int port=start_port; port <=end_port; port++)
                      {
			if (scan_port(ip,port))
                          {
				printf(" %-8d%-10s%s\n",port,"OPEN",port_name(port));
				open_count++;
			   }
		       }
                 /*output all ports*/
                printf(" ----------------------------------\n");
		printf(" %d open ports(s) found\n\n",open_count);
		return 0;

         }
	         
