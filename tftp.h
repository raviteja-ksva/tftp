#include <stdio.h>		/*for input and output */
#include <string.h>		/*for string functions */
#include <stdlib.h> 		/**/
#include <unistd.h> 		/**/
#include <sys/types.h>		/*used for the socket function */
#include <sys/socket.h>		/*for socket functions */
#include <netdb.h>		/*for address information and conversions, also holds the hostent structure */
#include <netinet/in.h>		/*for internet addresses */
#include <arpa/inet.h> 		/**/
#include <errno.h>
#include <sys/wait.h>
#include <dirent.h>

#define ERROR -1
#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 0x04
#define ERR 0x05

/*amount of time to wait for an ACK/Data Packet in 1000microseconds 1000 = 1 second*/
#define TIMEOUT 2000

/* Number of times to resend a data OR ack packet beforing giving up */
#define RETRIES 3

/* Maximum number of packets before ack */
#define MAXACKFREQ 16

#define MAXDATASIZE 1024 /* Maximum data size allowed */

/*a buffer for the messages passed between the client and the server */
static char buf[BUFSIZ];

char err_msg [7][40] = {"Not defined, see error message if any",
                        "File not fount",
                        "Access Violation",
                        "Disk full, or allocation exceeded",
                        "Illegal TFTP operation",
                        "Unknown transfer ID",
                        "File already exists"};
