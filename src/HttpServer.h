#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <getopt.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else /* !_WIN32 */
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif /* _WIN32 */
#include <signal.h>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/listener.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

#ifdef _WIN32
#include <event2/thread.h>
#endif /* _WIN32 */

#ifdef EVENT__HAVE_NETINET_IN_H
#include <netinet/in.h>
# ifdef _XOPEN_SOURCE_EXTENDED
#  include <arpa/inet.h>
# endif
#endif
#include "XLog.h"

struct options {
	int port=8080;
	int iocp;
	int verbose;
	int max_body_size=10000;

	int unlink;
	const char* unixsock;
	const char* bind="0.0.0.0";
	const char* docroot;
};

static const struct table_entry {
	const char* extension;
	const char* content_type;
} content_type_table[] = {
	{ "txt", "text/plain" },
	{ "c", "text/plain" },
	{ "h", "text/plain" },
	{ "html", "text/html" },
	{ "htm", "text/htm" },
	{ "css", "text/css" },
	{ "gif", "image/gif" },
	{ "jpg", "image/jpeg" },
	{ "jpeg", "image/jpeg" },
	{ "png", "image/png" },
	{ "pdf", "application/pdf" },
	{ "ps", "application/postscript" },
	{ NULL, NULL },
};

class HttpServer
{
public:
	HttpServer();
	~HttpServer();

	bool run();

private:
	static char uri_root[512];
	static void do_term(evutil_socket_t sig, short events, void* arg);
	static const char* guess_content_type(const char* path);
	static int display_listen_sock(evhttp_bound_socket* handle);
	static void dump_request_cb(evhttp_request* req, void* arg);
	static void send_document_cb(evhttp_request* req, void* arg);
	std::thread m_thread;
	event_config* cfg = NULL;
	event_base* base = NULL;
	evhttp* http = NULL;
	evhttp_bound_socket* handle = NULL;
	evconnlistener* lev = NULL;
	event* term = NULL;
	options o;
	int ret = 0;
};

