#include "tftp.h"

/*a function to print the Help menu*/
void help (char *);

/*a function to create the request packet, read or write*/
int req_packet (int opcode, char *filename, char *mode, char buf[]);
/*a function to creat an ACK packet*/
int ack_packet (int block, char buf[]);
/*a function to create the Error packets*/
int err_packet (int err_code, char *err_msg, char buf[]);
/*a function that will print the ip:port pair of the server or client, plus data sent or recieved*/
void ip_port (struct sockaddr_in host);
/*a function to send a file to the server*/
void tsend (char *pFilename, struct sockaddr_in server, char *pMode,
		int sock);
/*a function to get a file from the server*/
void tget (char *pFilename, struct sockaddr_in server, char *pMode, int sock);


/* default values which can be controlled by command line */
char path[64] = "/home/ravi/tftp-1.0/";
int port = 71;
unsigned short int ackfreq = 1;
int datasize = 512;
int debug = 1, w_size = 1, p_length = 512;


int main (int argc, char **argv)
{
	int sock, server_len, len, opt;
	char opcode, filename[196], mode[12] = "octet";
	struct hostent *host;
	struct sockaddr_in server;
	char* host_ip = "127.0.0.1";
	FILE *fp;

	if (argc < 2)
	{
		printf("invalid format of command\n");
		return 0;
	}

	while ((opt = getopt (argc, argv, "h:P:p:g:l:w:")) != -1)
	{
		// printf("inside first while \n");
		switch (opt)
		{
			case 'h':
				host_ip = optarg;
				printf("server ip is %s\n", host_ip);
				break;			
			case 'P':		// get port number
				port = atoi (optarg);
				printf ("Client: The port number is: %d\n", port);
				break;
			case 'p':
//				printf("need to put a file in server\n\n");
				strncpy (filename, optarg, sizeof (filename) - 1);
				opcode = WRQ;
				// fp = fopen (filename, "r");	
				// if (fp == NULL)
				// {
				// 	printf ("Client: file could not be opened\n");
				// 	return 0;
				// }
				// printf ("Client: The file name is: %s and can be read", filename);
				// fclose (fp);
				break;

			case 'g':
				strncpy (filename, optarg, sizeof (filename) - 1);
				opcode = RRQ;
				// fp = fopen (filename, "w");	/*opened the file for writting */
				// if (fp == NULL)
				// {
				// 	printf ("Client: file could not be created\n");
				// 	return 0;
				// }
				// printf ("Client: The file name is: %s and it has been created", filename);
				// fclose (fp);
				break;

			// case 'w':		/* Get the window size */
			// 	ackfreq = atoi (optarg);
			// 	if (debug)
			// 	{
			// 		printf ("Client: Window size is: %i\n", ackfreq);
			// 	}
			// 	//ackfreq = atoi (optarg);
			// 	if (ackfreq > MAXACKFREQ)
			// 	{
			// 		printf
			// 			("Client: Sorry, you specified an ack frequency higher than the maximum allowed (Requested: %d Max: %d)\n",
			// 			 ackfreq, MAXACKFREQ);
			// 		return 0;
			// 	}
			// 	else if (w_size == 0)
			// 	{
			// 		printf ("Client: Sorry, you have to ack sometime.\n");
			// 		return 0;
			// 	}
			// 	break;

			// case 'l':		/* packet length */
			// 	datasize = atoi (optarg);
			// 	if (debug)
			// 	{
			// 		printf ("Client: Packet length is: %i bytes\n", datasize);
			// 	}
			// 	if (datasize > MAXDATASIZE)
			// 	{
			// 		printf
			// 			("Client: Sorry, you specified a data size higher than the maximum allowed (Requested: %d Max: %d)\n",
			// 			 datasize, MAXDATASIZE);
			// 		return 0;
			// 	}
			// 	break;

			// case 'H':		/* Help (no opts) */
			// 	help (argv[0]);
			// 	return (0);
			// 	break;
			case 'o':
				strncpy (mode, "octet", sizeof (mode) - 1);
				printf ("Client: The mode is set to octet\n");
				break;
			case 'n':
				strncpy (mode, "netascii", sizeof (mode) - 1);
				printf ("Client: The mode is set to netascii\n");
				break;
			default:
				printf("invalid option in command.Please refer to syntax of command\n");
				return (0);
				break;
		}
	}

	// printf("outside while \n");
	if (!(host = gethostbyname (host_ip)))
	{
		perror ("could not obtain host address as");
		exit (2);
	}

	printf("opening file in required mode \n");
	if(opcode == WRQ)
		fp = fopen (filename, "r");	
	else if(opcode == RRQ)
		fp = fopen (filename, "w");		
	if (fp == NULL){
		printf ("Client: file could not be opened\n");
		return 0;
	}
	if(opcode == WRQ)
		printf ("Client: The file name is: %s and can be read\n", filename);
	else if(opcode == RRQ)
		printf ("Client: The file name is: %s and it has been created\n", filename);
	fclose (fp);		


	/*Create the socket, a -1 will show us an error */
	if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		printf ("client: socket");
		return 0;
	}

	 // bind to an arbitrary return address 
	 // because this is the client side, we don't care about the 
	 // address since no application will connect here  
	 // INADDR_ANY is the IP address and 0 is the socket 
	 // htonl converts a long integer (e.g. address) to a network 
	 // representation (agreed-upon byte ordering 

	bzero(&server, sizeof (server));
	server.sin_family = AF_INET;
	memcpy (&server.sin_addr, host->h_addr, host->h_length);
	// server.sin_addr.s_addr = htonl (INADDR_ANY);
	server.sin_port = htons (port);	


	server_len = sizeof (server);
	//printf("value os BUFSIZ is %d \n", BUFSIZ);
	memset (buf, 0, BUFSIZ);
	// creates RRQ packet
	len = req_packet (opcode, filename, mode, buf);

	if (sendto (sock, buf, len, 0, (struct sockaddr *) &server, server_len) != len)
	{
		perror ("Client: sendto has returend an error");
		exit(-1);
	}
	if (debug)
		ip_port (server);
	switch (opcode)
	{
		case RRQ:
			tget (filename, server, mode, sock);
			break;
		case WRQ:
			tsend (filename, server, mode, sock);
			break;
		default:
			printf ("Invalid opcode. Packet discarded.");
	}
	close (sock);
	return 1;
}

// void help (char *app)
// {
// 	printf
// 		("Usage:\n%s server [-h] [-d] [-P port] [-g] | [-p] [file-name] [-w size] [-l length] [-o] [-n]\n",
// 		 app);
// 	printf
// 		("Options:\n-h (help; this message)\n-d (Debug mode)\n-P port(Port number default is 69)\n-g (get a file from the server)\n-p (send a file to the server)\n");
// 	printf
// 		("-w size (set window size, default is 1)\n-l len (set max packet length, default is 512 bytes)\n");
// 	printf
// 		("-o for octet file transfer (default).\n-n for netascii file transfer\n");
// }


// creates a request packet and returns lenght of packet
int req_packet (int opcode, char *filename, char *mode, char header[])
{
	int lenght;
	// format is for RRQ/WRQ : | 01/02 | Filename | 0 | Mode | 0
	lenght = sprintf (header, "%c%c%s%c%s%c", 0x00, opcode, filename, 0x00, mode, 0x00);
	if (lenght == 0)
	{
		printf ("Error in creating the request packet\n");	/*could not print to the client buffer */
		exit(-1);
	}
	printf ("RRQ packet created.\n");
	return lenght;
}


// creates a request packet and returns length of packet
int ack_packet (int block, char buf[])
{
	int packet_len;
	packet_len = sprintf (buf, "%c%c%c%c", 0x00, ACK, 0x00, 0x00);
	buf[2] = (block & 0xFF00) >> 8;
	buf[3] = (block & 0x00FF);
	if (packet_len == 0)
	{
		printf ("Error in creating the ACK packet\n");
		exit(-1);
	}
	printf ("ACK packet created.\n");
	return packet_len;
}


/* A function that will create an error packet based on the error code*/
	int
err_packet (int err_code, char *err_msg, char buf[])
{
	int packet_len;
	int size = sizeof(buf);
	memset (buf, 0, size);
	packet_len =
		sprintf (buf, "%c%c%c%c%s%c", 0x00, ERR, 0x00, err_code, err_msg, 0x00);
	if (packet_len == 0)
	{
		printf ("Error in creating the ACK packet\n");	/*could not print to the client buffer */
		exit (ERROR);
	}
	printf ("ERROR packet created.\n");
	return packet_len;
}

void ip_port (struct sockaddr_in host)
{
	printf ("The IP:port for the host is: IP:%s Port:%d \n", inet_ntoa (host.sin_addr), ntohs (host.sin_port));
}

/*
 *This function is called when the client would like to upload a file to the server.
 */
void tsend (char *pFilename, struct sockaddr_in server, char *pMode, int sock)
{
	int len, server_len, opcode, ssize = 0, n, i, j, bcount = 0, tid;
	unsigned short int count = 0, rcount = 0, acked = 0;
	unsigned char filebuf[MAXDATASIZE + 1];
	unsigned char packetbuf[MAXACKFREQ][MAXDATASIZE + 12], recvbuf[MAXDATASIZE + 12];
	char filename[128], mode[12], *bufindex;	//fullpath[196],
	struct sockaddr_in ack;

	FILE *fp;			/* pointer to the file we will be sending */

	strcpy (filename, pFilename);	//copy the pointer to the filename into a real array
	strcpy (mode, pMode);		//same as above

	fp = fopen (filename, "r");
	if (fp == NULL)
	{				//if the pointer is null then the file can't be opened - Bad perms OR no such file
		printf ("Client: file - %s not found or permissions denied\n", filename);
		return;
	}
	else
		printf ("Client: PUTting file - %s\n", filename);

	//get ACK for WRQ
	/* The following 'for' loop is used to recieve/timeout ACKs */
	for (j = 0; j < RETRIES - 2; j++)
	{
		server_len = sizeof(ack);
		errno = EAGAIN;
		n = -1;
		for (i = 0; errno == EAGAIN && i <= TIMEOUT && n < 0; i++)
		{
			n=recvfrom(sock, recvbuf, sizeof(recvbuf), MSG_DONTWAIT, (struct sockaddr *) &ack, (socklen_t *) &server_len);
			usleep (1000);
		}

		tid = ntohs (ack.sin_port);	//get the tid of the server.
		server.sin_port = htons (tid);	//set the tid for rest of the transfer

		if (n < 0 && errno != EAGAIN)
			printf("Client: could not receive from the server (errno: %d n: %d)\n", errno, n);
		//resend packet
		else if (n < 0 && errno == EAGAIN)
			printf("Client: Timeout waiting for ack (errno: %d n: %d)\n",errno, n);
			//resend packet
		else
		{			/*changed client to server here */
			if (server.sin_addr.s_addr != ack.sin_addr.s_addr)	/* checks to ensure send to ip is same from ACK IP */
			{
				printf("Client: Error recieving ACK (ACK from invalid address)\n");
				j--;		/* in this case someone else connected to our port. Ignore this fact and retry getting the ack */
				continue;
			}
			if (tid != ntohs (server.sin_port))	/* checks to ensure get from the correct TID */
			{
				printf("Client: Error recieving file (data from invalid tid)\n");
				len = err_packet (5, err_msg[5], buf);
				if (sendto (sock, buf, len, 0, (struct sockaddr *) &server, sizeof (server)) != len)	/* send the data packet */
					perror ("Client: sendto has returend an error");
				j--;
				continue;		/* we aren't going to let another connection spoil our first connection */
			}

			/* same in the code in the tget function */
			bufindex = (char *) recvbuf;	//start our pointer going
			if (bufindex++[0] != 0x00)
				printf ("Client: bad first nullbyte!\n");
			opcode = *bufindex++;
			rcount = *bufindex++ << 8;
			rcount &= 0xff00;
			rcount += (*bufindex++ & 0x00ff);
			if (opcode != 4 || rcount != count)	/* ack packet should have code 4 (ack) and should be acking the packet we just sent */
			{
				printf("Client: Remote host failed to ACK proper data packet # %d (got OP: %d Block: %d)\n",
						 count, opcode, rcount);
				/* sending error message */
				if (opcode > 5)
				{
					len = err_packet (4, err_msg[4], buf);
					if (sendto (sock, buf, len, 0, (struct sockaddr *) &server, sizeof (server)) != len)	/* send the data packet */
					{
						perror ("Client: sendto has returend an error");
					}
				}

			}
			else
			{
				printf("Client:ACK - %d received\n",rcount);
				break;
			}
		}			//end of else
		printf ("Client: ACK's lost. Resending complete.\n");
	}


	memset (filebuf, 0, sizeof (filebuf));
	while (1)
	{
		acked = 0;
		ssize = fread(filebuf, 1, datasize, fp);
		// if (debug)
		// {
		// 	printf
		// 		("The first data block has been read from the file and will be sent to the server\n");
		// 	printf ("The size read from the file is: %d\n", ssize);
		// }

		count++;			/* count number of datasize byte portions we read from the file */
		if (count == 1)		/* we always look for an ack on the FIRST packet */
			bcount = 0;
		else if (count == 2)	//  The second packet will always start our counter at 0. 
			bcount = 0;
		else
			bcount = (count - 2) % ackfreq;

		// format of data block is - | opcode-03 | Block # | Data |
		sprintf((char *) packetbuf[bcount], "%c%c%c%c", 0x00, 0x03, 0x00, 0x00);
		memcpy((char *) packetbuf[bcount] + 4, filebuf, ssize);
		len = 4 + ssize;
		packetbuf[bcount][2] = (count & 0xFF00) >> 8;	//fill in the count (top number first)
		packetbuf[bcount][3] = (count & 0x00FF);	//fill in the lower part of the count
		if (debug)
			printf ("Client: Sending packet # %04d (length: %d file chunk: %d)\n",
					count, len, ssize);
		/* send the data packet */
		if (sendto
				(sock, packetbuf[bcount], len, 0, (struct sockaddr *) &server,
				 sizeof (server)) != len)
		{
			if (debug)
				printf ("Client: Mismatch in number of sent bytes\n");
			return;
		}
		if (debug)
		{
			ip_port (server);
			printf ("==count: %d  bcount: %d  ssize: %d  datasize: %d\n", count,
					bcount, ssize, datasize);
		}
		//if ((count - 1) == 0 || ((count - 1) % ackfreq) == 0 || ssize != datasize)
		if (((count) % ackfreq) == 0 || ssize != datasize)
		{
			if (debug)
				printf ("-- I will get an ACK\n");
			/* The following 'for' loop is used to recieve/timeout ACKs */
			for (j = 0; j < RETRIES; j++)
			{
				server_len = sizeof (ack);
				errno = EAGAIN;
				n = -1;
				for (i = 0; errno == EAGAIN && i <= TIMEOUT && n < 0; i++)
				{
					n =
						recvfrom (sock, recvbuf, sizeof (recvbuf), MSG_DONTWAIT,
								(struct sockaddr *) &ack,
								(socklen_t *) & server_len);
					/* if (debug)
					   ip_port(ack); */
					usleep (1000);
				}
				if (n < 0 && errno != EAGAIN)
				{
					if (debug)
						printf
							("Client: could not receive from the server (errno: %d n: %d)\n",
							 errno, n);
					//resend packet
				}
				else if (n < 0 && errno == EAGAIN)
				{
					if (debug)
						printf
							("Client: Timeout waiting for ack (errno: %d n: %d)\n",
							 errno, n);
					//resend packet
				}
				else
				{		/* checks to ensure send to ip is same from ACK IP */
					if (server.sin_addr.s_addr != ack.sin_addr.s_addr)
					{
						if (debug)
							printf
								("Client: Error recieving ACK (ACK from invalid address)\n");
						/* in this case someone else connected to our port. Ignore this fact and retry getting the ack */
						j--;
						continue;
					}
					if (tid != ntohs (server.sin_port))	/* checks to ensure get from the correct TID */
					{
						if (debug)
							printf
								("Client: Error recieving file (data from invalid tid)\n");
						len = err_packet (5, err_msg[5], buf);
						/* send the data packet */
						if (sendto
								(sock, buf, len, 0, (struct sockaddr *) &server,
								 sizeof (server)) != len)
						{
							printf
								("Client: Mismatch in number of sent bytes while trying to send mode error packet\n");
						}
						/*if (debug)
						  ip_port(server);  */
						j--;

						continue;	/* we aren't going to let another connection spoil our first connection */
					}

					/* this formatting code is just like the code in the main function */
					bufindex = (char *) recvbuf;	//start our pointer going
					if (bufindex++[0] != 0x00)
						printf ("Client: bad first nullbyte!\n");
					opcode = *bufindex++;

					rcount = *bufindex++ << 8;
					rcount &= 0xff00;
					rcount += (*bufindex++ & 0x00ff);
					if (opcode != 4 || rcount != count)	/* ack packet should have code 4 (ack) and should be acking the packet we just sent */
					{
						if (debug)
							printf
								("Client: Remote host failed to ACK proper data packet # %d (got OP: %d Block: %d)\n",
								 count, opcode, rcount);
						/* sending error message */
						if (opcode > 5)
						{
							len = err_packet (4, err_msg[4], buf);
							if (sendto (sock, buf, len, 0, (struct sockaddr *) &server, sizeof (server)) != len)	/* send the data packet */
							{
								printf
									("Client: Mismatch in number of sent bytes while trying to send mode error packet\n");
							}
							/*if (debug)
							  ip_port(server); */
						}
						/* from here we will loop back and resend */
					}
					else
					{
						if (debug)
							printf
								("Client: Remote host successfully ACK'd (#%d)\n",
								 rcount);
						break;
					}
				}
				for (i = 0; i <= bcount; i++)
				{
					if (sendto (sock, packetbuf[i], len, 0, (struct sockaddr *) &server, sizeof (server)) != len)	/* resend the data packet */
					{
						if (debug)
							printf ("Client: Mismatch in number of sent bytes\n");
						return;
					}
					if (debug)
					{
						printf ("Client: Ack(s) lost. Resending: %d\n",
								count - bcount + i);
						ip_port (server);
					}
				}
				if (debug)
					printf ("Client: Ack(s) lost. Resending complete.\n");

			}
			/* The ack sending 'for' loop ends here */

		}
		else if (debug)
		{
			printf
				("Client: Not attempting to recieve ack. Not required. count: %d\n",
				 count);
			n = recvfrom (sock, recvbuf, sizeof (recvbuf), MSG_DONTWAIT, (struct sockaddr *) &ack, (socklen_t *) & server_len);	/* just do a quick check incase the remote host is trying with ackfreq = 1 */
			/*if (debug)
			  ip_port(ack); */
		}

		if (j == RETRIES)
		{
			if (debug)
				printf ("Client: Ack Timeout. Aborting transfer\n");
			fclose (fp);

			return;
		}
		if (ssize != datasize)
			break;

		memset (filebuf, 0, sizeof (filebuf));	/* fill the filebuf with zeros so that when the fread fills it, it is a null terminated string */
	}

	fclose (fp);
	if (debug)
		printf ("Client: File sent successfully\n");

	return;
}				//end of tsend function



// get file from server

void tget (char *p_Filename, struct sockaddr_in server, char *p_Mode, int sock)
{
	/* local variables */
	int len, server_len, opcode, i, j, n, tid = 0, flag = 1;
	unsigned short int count = 0, rcount = 0;
	unsigned char filebuf[MAXDATASIZE + 1];
	unsigned char packetbuf[MAXDATASIZE + 12];
	extern int errno;
	char filename[128], mode[12], *bufindex, ackbuf[512];
	struct sockaddr_in data;
	FILE *fp;			/* pointer to the file we will be getting */

	strcpy (filename, p_Filename);
	strcpy (mode, p_Mode);


	// open the file for writing since this is in get file
	fp = fopen (filename, "w");	
	if (fp == NULL)
	{
		printf("client: unable to open requested file- (%s)\n", filename);
		return;
	}
	else
		printf("file %s created \n", filename);

	bzero(filebuf, sizeof(filebuf));

	// datasize = 512 i.e, payload size
	// add 4 bytes for opcode and block number
	n = datasize + 4;
	do
	{
		bzero(packetbuf, sizeof(packetbuf));
		bzero(ackbuf, sizeof(ackbuf));
		// if datasize < full packet => this is last packet to be received 
		if (n != (datasize + 4))	
		{
			printf("last packet identified payload size is %d \n", n-4);
			// Last packet's ACK should have
			// opcode - 04 and block number - 00 
			len = sprintf (ackbuf, "%c%c%c%c", 0x00, 0x04, 0x00, 0x00);

			ackbuf[2] = (count & 0xFF00) >> 8;	//fill in the count (top number first)
			ackbuf[3] = (count & 0x00FF);	//fill in the lower part of the count
			// printf ("sending ACK %04d\n", count);

			if (sendto(sock, ackbuf, len, 0, (struct sockaddr *) &server, sizeof (server)) != len)
			{
				perror("Client: sendto has returend an error");
				return;
			}
			printf ("ACK %04d sent\n", count);
			goto done;
		}

		count++;

		// loop until we get correct ack OR timed out
		for (j = 0; j < RETRIES; j++)
		{
			server_len = sizeof (data);
			errno = EAGAIN;	// 	i.e., Resource temporarily unavailable
			n = -1;
			//  this for loop will be checking the non-blocking socket until timeout 
			for (i = 0; errno == EAGAIN && i <= TIMEOUT && n < 0; i++)
			{
				n =	recvfrom (sock, packetbuf, sizeof (packetbuf) - 1,
							MSG_DONTWAIT, (struct sockaddr *) &data, (socklen_t *) & server_len);
				usleep (1000);
			}
			if (!tid)
			{
				tid = ntohs (data.sin_port);	//get the tid of the server.
				server.sin_port = htons (tid);	//set the tid for rest of the transfer 
			}

			// means there is an error that is not timed out
			if (n < 0 && errno != EAGAIN)
				printf("The server could not receive from the client (errno: %d n: %d)\n",errno, n);

			// means recvfrom timed out
			else if (n < 0 && errno == EAGAIN)
				printf("Timeout waiting for data (errno: %d == %d n: %d)\n", errno, EAGAIN, n);

			else
			{
				// if (server.sin_addr.s_addr != data.sin_addr.s_addr)	/* checks to ensure get from ip is same from ACK IP */
				// {
				// 	if (debug)
				// 		printf
				// 			("Error recieving file (data from invalid address)\n");
				// 	j--;
				// 	continue;	/* we aren't going to let another connection spoil our first connection */
				// }
				if (tid != ntohs (server.sin_port))	/* checks to ensure get from the correct TID */
				{
					printf ("Error recieving file sending error packet\n");
					// send error packet - opcode: 5
					len = sprintf((char *) packetbuf, "%c%c%c%cBad/Unknown TID%c",0x00, 0x05, 0x00, 0x05, 0x00);
					if (sendto (sock, packetbuf, len, 0, (struct sockaddr *) &server, sizeof (server)) != len)
						perror("Client: sendto has returend an error");
					j--;
					continue;
				}

				bufindex = (char *) packetbuf;
				if (bufindex++[0] != 0x00)
					printf ("bad first nullbyte!\n");
				opcode = *bufindex++;
				rcount = *bufindex++ << 8;
				rcount &= 0xff00;
				rcount += (*bufindex++ & 0x00ff);

				// copy the rest of the packet (data portion) into our data array
				memcpy ((char *) filebuf, bufindex, n - 4);	
				printf("received packet- %d\n", rcount);
				if (flag)
				{
					if (n > 516)
						datasize = n - 4;
					else if (n < 516)
						datasize = 512;
					flag = 0;
				}
				if (opcode != 3)	/* ack packet should have code 3 (data) and should be ack+1 the packet we just sent */
				{
					printf("invalid data packet (Got OP: %d Block: %d) (Wanted Op: 3 Block: %d)\n", opcode, rcount, count);
					/* send error message */
					if (opcode > 5)
					{
						len =sprintf((char *) packetbuf,"%c%c%c%cIllegal operation%c",0x00, 0x05, 0x00, 0x04, 0x00);
						if (sendto (sock, packetbuf, len, 0, (struct sockaddr *) &server, sizeof (server)) != len)
							printf("Mismatch in number of sent bytes while trying to send mode error packet\n");
					}
				}
				else
				{
					// ACK opcode: 4 , expected block #
					len = sprintf (ackbuf, "%c%c%c%c", 0x00, 0x04, 0x00, 0x00);
					ackbuf[2] = (count & 0xFF00) >> 8;	//fill in the count (top number first)
					ackbuf[3] = (count & 0x00FF);	//fill in the lower part of the count
					// printf ("ACK %04d sending\n", count);
					if (((count - 1) % ackfreq) == 0)
					{
						if (sendto(sock, ackbuf, len, 0, (struct sockaddr *) &server, sizeof (server)) != len)
						{
							perror("Client: sendto has returend an error");
							return;
						}
						printf ("ACK %04d sent\n", count);
					}		//check for ackfreq
					else if (count == 1)
					{
						if (sendto(sock, ackbuf, len, 0, (struct sockaddr *) &server, sizeof (server)) != len)
						{
							printf ("Mismatch in number of sent bytes\n");
							return;
						}
						printf ("ACK 1 sent\n");
					}
					break;
				}		//end of else
			}
		}
		if (j == RETRIES)
		{
			printf ("Receive time Out\n");
			fclose (fp);
			return;
		}
	}
	// if it doesn't write the file the length of the packet received less 4 then it didn't work 
	while (fwrite (filebuf, 1, n - 4, fp) == n - 4);
	fclose (fp);
	sync ();
	printf("file failed to recieve properly\n");
	return;

done:

	fclose (fp);
	sync ();
	if (debug)
		printf ("File received successfully\n");
	return;
}
