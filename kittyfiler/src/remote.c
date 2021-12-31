#include <poll.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#ifndef APP_BUFFERSIZE
#define APP_BUFFERSIZE 128
#endif /* #ifndef APP_BUFFERSIZE */

static char ipbuffer[APP_BUFFERSIZE];
static char portbuffer[APP_BUFFERSIZE];
static int fd = -1;

static int remoteConnect()
{
	int s; /* Socket */
	int error;

	struct addrinfo hints = {
		.ai_flags = AI_PASSIVE,
		.ai_family = 0,
		.ai_socktype = SOCK_DGRAM,
		.ai_protocol = IPPROTO_UDP,
		.ai_addrlen = 0,
		.ai_canonname = NULL,
		.ai_addr = NULL,
		.ai_next = NULL
	};

	struct addrinfo* sockai;
	struct addrinfo* ai_iter;

	if ((error = getaddrinfo(ipbuffer, portbuffer, &hints, &sockai)) != 0) {
		fprintf(stderr, "Failed to look up address %s:%s with code %d: "
			"%s", ipbuffer, portbuffer, error, gai_strerror(error));
		return -1;
	}

	assert(sockai);
	ai_iter = sockai;

	do {
		if ((s = socket(ai_iter->ai_family, ai_iter->ai_socktype,
				ai_iter->ai_protocol)) < 0) {
			continue;
		}

		if (connect(s, ai_iter->ai_addr, ai_iter->ai_addrlen) == 0) {
			break;
		}

		s = -1;
	}
	while ((ai_iter = ai_iter->ai_next) != NULL);

	freeaddrinfo(sockai);

	return s;
}

int connectDeviceUDP(const char* address, const char* port)
{
	/* Since ip and port are shared, this isn't thread safe */
	strncpy(ipbuffer, address, APP_BUFFERSIZE - 1);
	ipbuffer[APP_BUFFERSIZE - 1] = '\0';
	strncpy(portbuffer, port, APP_BUFFERSIZE - 1);
	portbuffer[APP_BUFFERSIZE - 1] = '\0';

	if ((fd = remoteConnect()) < 0) {
		fprintf(stderr, "Failed to open %s:%s\n",
			ipbuffer, portbuffer);
		return 1;
	}

	/* Attempt to write to remote to start data flow */
	if (write(fd, "hello\n", 6) < 0) {
		fprintf(stderr, "Failed to write to address %s:%s:"
			" %s\n", ipbuffer, portbuffer,
			strerror(errno));
		return 1;
	}

	return 0;
}

/// Check for data at device and read to given buffer
/// Returns 0 on success, -1 on timeout, and 1 on failure
int pollDeviceRead(char* buf, unsigned int len, int timeout)
{
	int pollresult;
	int readresult;
	struct pollfd pfd = {
		.fd = fd,
		.events = POLLIN
	};

	if (fd < 0) {
		fprintf(stderr, "Error: Connection not initialized");
		return 1;
	}

	pollresult = poll(&pfd, 1, timeout);

	if (pollresult < 0) {
		perror("Critical Error polling file: ");
		return 1;
	}

	if (pollresult == 0) {
		return -1;
	}

	if ((readresult = read(fd, buf, len)) < 1) {
		perror("Error: Read failed with: ");
		return 1;
	}

	buf[readresult] = '\0';
	return 0;
}

int disconnectDeviceUDP()
{
	if (close(fd) < 0) {
		perror("Error: Failed to close UDP connector: ");
		return 1;
	}

	return 0;
}
