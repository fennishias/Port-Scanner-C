# Port-Scanner-C
# Building a Raw Socket Port Scanner in C 


How I went from wrapping Nmap in a C program to writing my own TCP connect scanner using raw sockets


---

## Why I Built This

I wanted to understand how port scanners actually work under the hood. Most tutorials just show you how to **run** Nmap — but I wanted to know what Nmap itself is doing. So I started with a simple C wrapper around Nmap, then rewrote it from scratch using raw sockets.

 *"Your C program never touches a single network packet when wrapping Nmap. It's purely a smart interface that assembles the right command and fires it off."*

---

##  Version 1 — Wrapping Nmap in C

It reads a target IP and port range from the user, builds an Nmap command string with `sprintf()`, and runs it with `system()`.

```c
#include <stdio.h> //printf, fgets
#include <stdlib.h> //system()
#include <string.h> //strlen

int main(){
        char target[100]; //stores the IP/hostname the user types
        char ports[50];  //stores the port range
        char command[300]; //stores the final nmap command


        //This asks the user for a target
        printf("Enter target IP or hostname:");
        fgets(target, sizeof(target),stdin);

        //This removes the newline character that fgets add at the end
        target[strlen(target) -1] = '\0';

        //This asks the user for ports to scan
        printf("Enter port range (1 - 10000):");
        fgets(ports,sizeof(ports),stdin);

        //This removes the newline character 
        ports[strlen(ports) -1] ='\0';

        //Here we build an nmap scan command
        sprintf(command, "nmap -p %s %s", ports,target);

        //This run the command
        system(command);

        return 0;

}


```

**Compile and run:**
```bash
gcc -o scanner port_scanner.c
sudo ./scanner
```

---

## Version 2 — Raw Sockets (No Nmap)

The real version uses `socket()`, `connect()`, and `setsockopt()` from the Linux socket API. For each port, it tries a full TCP handshake — if the connection succeeds, the port is open.I tried with few ports.

### How it works step by step

| Step | Function | What it does |
|------|----------|--------------|
| 1 | `socket()` | Opens a TCP socket — like picking up a phone before dialling |
| 2 | `setsockopt()` | Sets a 1-second timeout so filtered ports don't hang forever |
| 3 | `connect()` | Returns `0` if open, `-1` if closed or filtered |
| 4 | `close()` | Cleans up the file descriptor after every check |

### The full code

```c
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
/* scan_port() - tries to TCP-connect to ip:port, return 1 if open and 0 if closed/filtered */

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
       
```

**Compile and run:**
```bash
gcc -o Mscanner raw_scanner.c
./Mscanner
```

**Example output:**
```
  === Mariquita Port Scanner ===

  Target IP: 192.168.1.1
  Start port: 1
  End port:   1000

  Scanning 192.168.1.1  ports 1-1000 ...
  PORT     STATE      SERVICE
  ----------------------------------------
  22       OPEN       SSH
  80       OPEN       HTTP
  443      OPEN       HTTPS
  ----------------------------------------
  3 open port(s) found.
```

---

## What I Learned

Writing a port scanner from scratch taught me more about networking than any tutorial. The key things that clicked:

**Every network service is just a process listening on a port.** When `connect()` succeeds, something on the other end accepted the handshake. When it fails with `ECONNREFUSED`, the OS itself rejected it. When it times out, a firewall silently dropped the packet.

**The difference between our scanner and Nmap** is that Nmap does a SYN scan — it sends only the first packet of the TCP handshake and never completes the connection. That requires root and is faster and stealthier. Our version does a full connect scan which works without root but is slightly more detectable.

---

##  Legal Notice

Only scan systems you own or have **explicit written permission** to test. Unauthorized port scanning may be illegal in your country.

---

## 📋 Requirements

- GCC compiler
- Linux (tested on Kali Linux)
- No external libraries needed

```bash
sudo apt install gcc    # if not already installed
```
Can work both on linux and MacOs systesms.
---

*Built on Kali Linux · Written in C · No external dependencies*
