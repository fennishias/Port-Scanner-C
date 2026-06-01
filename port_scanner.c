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


