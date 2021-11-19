#include "webserver.h"

#include <string.h>

#include "lwip/netif.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "xil_printf.h"

extern void print_ip(char *msg, ip_addr_t *ip);

extern struct netif server_netif;

void process_http_request(int sd)
{
	int read_len;
	int RECV_BUF_SIZE = 1400;  /* http request size can be a max of RECV_BUF_SIZE */
	char recv_buf[RECV_BUF_SIZE];

	/* read in the request */
	if ((read_len = read(sd, recv_buf, RECV_BUF_SIZE)) <= 0) {
		close(sd);
		vTaskDelete(NULL);
		return;
	}

	/* respond to request */
	generate_response(sd, recv_buf, read_len);

	/* close connection */
	close(sd);
}

void start_application()
{
	int sock, new_sd;
	int size = sizeof(struct sockaddr_in);
	struct sockaddr_in address, remote;

	/* initialize FS */
	if (platform_init_fs()) {
		xil_printf("Can't run webserver as FS init failed\r\n");
		return;
	}

	/* create a TCP socket */
	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		xil_printf("Failed to create socket\r\n");
		return;
	}

	/* bind to port 80 at any interface */
	address.sin_family = AF_INET;
	address.sin_port = htons(HTTP_PORT);
	address.sin_addr.s_addr = INADDR_ANY;
	if (lwip_bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
		xil_printf("Failed to bind at port '%d'\r\n", HTTP_PORT);
		return;
	}

	/* listen for incoming connections */
	if (lwip_listen(sock, 0)) {
		xil_printf("Failed to listen\r\n");
		return;
	}

	while (1) {
		new_sd = lwip_accept(sock, (struct sockaddr *)&remote,
					(socklen_t *)&size);
		process_http_request(new_sd);
	}
}

void print_app_header()
{
	xil_printf("http server is running on port %d\r\n", HTTP_PORT);
	print_ip("Please point your web browser to http://",
		&server_netif.ip_addr);
	xil_printf("\r\n");
}
