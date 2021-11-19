#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#define MAX_FILENAME 256
#define HTTP_PORT    80

/* initialize file system layer */
int platform_init_fs();

/* initialize device layer */
int http_init_devices();

/* web_utils.c utilities */
void extract_file_name(char *filename, char *req, int rlen, int maxlen);

char *get_file_extension(char *buf);
int is_cmd_print(char *buf);

int generate_response(int sd, char *http_req, int http_req_len);
int generate_http_header(char *buf, char *fext, int fsize);

#endif
