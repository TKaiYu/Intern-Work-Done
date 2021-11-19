#include <string.h>
#include <stdio.h>

#include "webserver.h"

int is_cmd_print(char *buf)
{
	/* skip past 'POST /' */
	buf += 6;

	/* then check for cmd/printxhr */
	return (!strncmp(buf, "cmd", 3) && !strncmp(buf + 4, "printxhr", 8));
}

void extract_file_name(char *filename, char *req, int rlen, int maxlen)
{
	char *fstart, *fend;
	int offset;

	/* first locate the file name in the request */
	/* requests are of the form GET /path/to/filename HTTP... */

	offset = strlen("GET ");

	if (req[offset] == '/')
		offset++;

	fstart = req + offset;   /* start marker */

	/* file name finally ends in a space */
	while (req[offset] != ' ')
		offset++;

	fend = req + offset - 1; /* end marker */

	if (fend < fstart) {
		strcpy(filename, "index.htm");
		return;
	}

	/* if there is something wrong with the URL & we ran for for more than
	 * the HTTP buffer length (offset > rlen) or the filename is too long,
	 * throw 404 error */
	if (offset > rlen || fend - fstart > maxlen) {
		*fend = 0;
		strcpy(filename, "404.htm");
		printf("Request filename is too long, length = %d, file = %s (truncated), max = %d\r\n",
				(int)(fend - fstart), fstart, maxlen);
		return;
	}

	/* copy over the filename */
	strncpy(filename, fstart, fend-fstart+1);
	filename[fend-fstart+1] = 0;

	/* if last character is a '/', append index.htm */
	if (*fend == '/')
		strcat(filename, "index.htm");
}

char *get_file_extension(char *fname)
{
	char *fext = fname + strlen(fname) - 1;

	while (fext > fname) {
		if (*fext == '.')
			return fext + 1;
		fext--;
	}

	return NULL;
}

int generate_http_header(char *buf, char *fext, int fsize)
{
	char lbuf[40];

	strcpy(buf, "HTTP/1.1 200 OK\r\nContent-Type: ");

	if (fext == NULL)
		strcat(buf, "text/html");	/* for unknown types */
	else if (!strncmp(fext, "htm", 3))
		strcat(buf, "text/html");	/* html */
	else if (!strncmp(fext, "jpg", 3))
		strcat(buf, "image/jpeg");
	else if (!strncmp(fext, "gif", 3))
		strcat(buf, "image/gif");
	else if (!strncmp(fext, "jsn", 3))
		strcat(buf, "application/json");
	else if (!strncmp(fext, "js", 2))
		strcat(buf, "text/javascript");
	else if (!strncmp(fext, "pdf", 2))
		strcat(buf, "application/pdf");
	else if (!strncmp(fext, "css", 2))
		strcat(buf, "text/css");
	else
		strcat(buf, "text/plain");	/* for unknown types */
	strcat(buf, "\r\n");

	sprintf(lbuf, "Content-length: %d", fsize);
	strcat(buf, lbuf);
	strcat(buf, "\r\n");

	strcat(buf, "Connection: close\r\n");
	strcat(buf, "\r\n");

	return strlen(buf);
}
