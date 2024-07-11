#include "HttpServer.h"
char HttpServer::uri_root[512] = {0};
HttpServer::HttpServer()
{
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		ret = 1;
		return;
	}
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	cfg = event_config_new();
	base = event_base_new_with_config(cfg);
	if (!base) {
		LOG(ERROR)("Couldn't create an event_base: exiting");
		ret = 1;
	}
	event_config_free(cfg);
	cfg = NULL;
	/* Create a new evhttp object to handle requests. */
	http = evhttp_new(base);
	if (!http) {
		LOG(ERROR)("couldn't create evhttp. Exiting.");
		ret = 1;
	}
	/* The /dump URI will dump all requests to stdout and say 200 ok. */
	evhttp_set_cb(http, "/dump", dump_request_cb, NULL);

	/* We want to accept arbitrary requests, so we need to set a "generic"
	 * cb.  We can also add callbacks for specific paths. */
	evhttp_set_gencb(http, send_document_cb, &o);
	if (o.max_body_size)
		evhttp_set_max_body_size(http, o.max_body_size);
	handle = evhttp_bind_socket_with_handle(http, o.bind, o.port);
	if (!handle) {
		LOG(ERROR)("couldn't bind to %s:%d. Exiting.", o.bind, o.port);
		ret = 1;
		return;
	}
	if (display_listen_sock(handle)) {
		ret = 1;
		return;
	}

	term = evsignal_new(base, SIGINT, do_term, base);
	if (!term)
		return;
	if (event_add(term, NULL))
		return;

}

HttpServer::~HttpServer()
{
	if (cfg)
		event_config_free(cfg);
	if (http)
		evhttp_free(http);
	if (term)
		event_free(term);
	if (base)
		event_base_free(base);
	if (m_thread.joinable()) {
		m_thread.join();
	}
}

bool HttpServer::run()
{
	if (ret == 1) {
		LOG(ERROR)("HttpServer start Fail!");
		return false;
	}
	m_thread = std::thread(event_base_dispatch, base);
	LOG(INFO)("HttpServer start success!");
	return true;
}

void HttpServer::do_term(evutil_socket_t sig, short events, void* arg)
{
	event_base* base = (event_base*)arg;
	event_base_loopbreak(base);
	fprintf(stderr, "Got %i, Terminating\n", sig);
}

/* Try to guess a good content-type for 'path' */
const char* HttpServer::guess_content_type(const char* path)
{
	const char* last_period, * extension;
	const struct table_entry* ent;
	last_period = strrchr(path, '.');
	if (!last_period || strchr(last_period, '/'))
		goto not_found; /* no exension */
	extension = last_period + 1;
	for (ent = &content_type_table[0]; ent->extension; ++ent) {
		if (!evutil_ascii_strcasecmp(ent->extension, extension))
			return ent->content_type;
	}

not_found:
	return "application/misc";
}

int HttpServer::display_listen_sock(struct evhttp_bound_socket* handle)
{
	struct sockaddr_storage ss;
	evutil_socket_t fd;
	ev_socklen_t socklen = sizeof(ss);
	char addrbuf[128];
	void* inaddr;
	const char* addr;
	int got_port = -1;

	fd = evhttp_bound_socket_get_fd(handle);
	memset(&ss, 0, sizeof(ss));
	if (getsockname(fd, (struct sockaddr*)&ss, &socklen)) {
		perror("getsockname() failed");
		return 1;
	}

	if (ss.ss_family == AF_INET) {
		got_port = ntohs(((struct sockaddr_in*)&ss)->sin_port);
		inaddr = &((struct sockaddr_in*)&ss)->sin_addr;
	}
	else if (ss.ss_family == AF_INET6) {
		got_port = ntohs(((struct sockaddr_in6*)&ss)->sin6_port);
		inaddr = &((struct sockaddr_in6*)&ss)->sin6_addr;
	}

	else {
		fprintf(stderr, "Weird address family %d\n",
			ss.ss_family);
		return 1;
	}

	addr = evutil_inet_ntop(ss.ss_family, inaddr, addrbuf,
		sizeof(addrbuf));
	if (addr) {
		printf("Listening on %s:%d\n", addr, got_port);
		evutil_snprintf(uri_root, sizeof(uri_root),
			"http://%s:%d", addr, got_port);
	}
	else {
		fprintf(stderr, "evutil_inet_ntop failed\n");
		return 1;
	}

	return 0;
}

/* Callback used for the /dump URI, and for every non-GET request:
 * dumps all information to stdout and gives back a trivial 200 ok */
void HttpServer::dump_request_cb(struct evhttp_request* req, void* arg)
{
	const char* cmdtype;
	struct evkeyvalq* headers;
	struct evkeyval* header;
	struct evbuffer* buf;

	switch (evhttp_request_get_command(req)) {
	case EVHTTP_REQ_GET: cmdtype = "GET"; break;
	case EVHTTP_REQ_POST: cmdtype = "POST"; break;
	case EVHTTP_REQ_HEAD: cmdtype = "HEAD"; break;
	case EVHTTP_REQ_PUT: cmdtype = "PUT"; break;
	case EVHTTP_REQ_DELETE: cmdtype = "DELETE"; break;
	case EVHTTP_REQ_OPTIONS: cmdtype = "OPTIONS"; break;
	case EVHTTP_REQ_TRACE: cmdtype = "TRACE"; break;
	case EVHTTP_REQ_CONNECT: cmdtype = "CONNECT"; break;
	case EVHTTP_REQ_PATCH: cmdtype = "PATCH"; break;
	default: cmdtype = "unknown"; break;
	}

	printf("Received a %s request for %s\nHeaders:\n",
		cmdtype, evhttp_request_get_uri(req));

	headers = evhttp_request_get_input_headers(req);
	for (header = headers->tqh_first; header;
		header = header->next.tqe_next) {
		printf("  %s: %s\n", header->key, header->value);
	}

	buf = evhttp_request_get_input_buffer(req);
	puts("Input data: <<<");
	while (evbuffer_get_length(buf)) {
		int n;
		char cbuf[128];
		n = evbuffer_remove(buf, cbuf, sizeof(cbuf));
		if (n > 0)
			(void)fwrite(cbuf, 1, n, stdout);
	}
	puts(">>>");

	evhttp_send_reply(req, 200, "OK", NULL);
}

/* This callback gets invoked when we get any http request that doesn't match
 * any other callback.  Like any evhttp server callback, it has a simple job:
 * it must eventually call evhttp_send_error() or evhttp_send_reply().
 */
void HttpServer::send_document_cb(struct evhttp_request* req, void* arg)
{
	evbuffer* evb = NULL;
	options* o = (options*)arg;
	const char* uri = evhttp_request_get_uri(req);
	struct evhttp_uri* decoded = NULL;
	const char* path;
	char* decoded_path;
	char* whole_path = NULL;
	size_t len;
	int fd = -1;
	struct stat st;

	if (evhttp_request_get_command(req) != EVHTTP_REQ_GET) {
		dump_request_cb(req, arg);
		return;
	}

	printf("Got a GET request for <%s>\n", uri);

	/* Decode the URI */
	decoded = evhttp_uri_parse(uri);
	if (!decoded) {
		printf("It's not a good URI. Sending BADREQUEST\n");
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return;
	}

	/* Let's see what path the user asked for. */
	path = evhttp_uri_get_path(decoded);
	if (!path) path = "/";

	/* We need to decode it, to see what path the user really wanted. */
	decoded_path = evhttp_uridecode(path, 0, NULL);
	if (decoded_path == NULL)
		goto err;
	/* Don't allow any ".."s in the path, to avoid exposing stuff outside
	 * of the docroot.  This test is both overzealous and underzealous:
	 * it forbids aceptable paths like "/this/one..here", but it doesn't
	 * do anything to prevent symlink following." */
	if (strstr(decoded_path, ".."))
		goto err;

	len = strlen(decoded_path) + strlen(o->docroot) + 2;
	if (!(whole_path = (char *)malloc(len))) {
		perror("malloc");
		goto err;
	}
	evutil_snprintf(whole_path, len, "%s/%s", o->docroot, decoded_path);

	if (stat(whole_path, &st) < 0) {
		goto err;
	}

	/* This holds the content we're sending. */
	evb = evbuffer_new();

	if (S_ISDIR(st.st_mode)) {
		/* If it's a directory, read the comments and make a little
		 * index page */
#ifdef _WIN32
		HANDLE d;
		WIN32_FIND_DATAA ent;
		char* pattern;
		size_t dirlen;
#else
		DIR* d;
		struct dirent* ent;
#endif
		const char* trailing_slash = "";

		if (!strlen(path) || path[strlen(path) - 1] != '/')
			trailing_slash = "/";

#ifdef _WIN32
		dirlen = strlen(whole_path);
		pattern = malloc(dirlen + 3);
		if (!pattern)
			goto err;
		memcpy(pattern, whole_path, dirlen);
		pattern[dirlen] = '\\';
		pattern[dirlen + 1] = '*';
		pattern[dirlen + 2] = '\0';
		d = FindFirstFileA(pattern, &ent);
		free(pattern);
		if (d == INVALID_HANDLE_VALUE)
			goto err;
#else
		if (!(d = opendir(whole_path)))
			goto err;
#endif

		evbuffer_add_printf(evb,
			"<!DOCTYPE html>\n"
			"<html>\n <head>\n"
			"  <meta charset='utf-8'>\n"
			"  <title>%s</title>\n"
			"  <base href='%s%s'>\n"
			" </head>\n"
			" <body>\n"
			"  <h1>%s</h1>\n"
			"  <ul>\n",
			decoded_path, /* XXX html-escape this. */
			path, /* XXX html-escape this? */
			trailing_slash,
			decoded_path /* XXX html-escape this */);
#ifdef _WIN32
		do {
			const char* name = ent.cFileName;
#else
		while ((ent = readdir(d))) {
			const char* name = ent->d_name;
#endif
			evbuffer_add_printf(evb,
				"    <li><a href=\"%s\">%s</a>\n",
				name, name);/* XXX escape this */
#ifdef _WIN32
		} while (FindNextFileA(d, &ent));
#else
		}
#endif
		evbuffer_add_printf(evb, "</ul></body></html>\n");
#ifdef _WIN32
		FindClose(d);
#else
		closedir(d);
#endif
		evhttp_add_header(evhttp_request_get_output_headers(req),
			"Content-Type", "text/html");
	}
	else {
		/* Otherwise it's a file; add it to the buffer to get
		 * sent via sendfile */
		const char* type = guess_content_type(decoded_path);
		if ((fd = open(whole_path, O_RDONLY)) < 0) {
			perror("open");
			goto err;
		}

		if (fstat(fd, &st) < 0) {
			/* Make sure the length still matches, now that we
			 * opened the file :/ */
			perror("fstat");
			goto err;
		}
		evhttp_add_header(evhttp_request_get_output_headers(req),
			"Content-Type", type);
		evbuffer_add_file(evb, fd, 0, st.st_size);
	}

	evhttp_send_reply(req, 200, "OK", evb);
	goto done;
err:
	evhttp_send_error(req, HTTP_NOTFOUND, NULL);
	if (fd >= 0)
		close(fd);
done:
	if (decoded)
		evhttp_uri_free(decoded);
	if (decoded_path)
		free(decoded_path);
	if (whole_path)
		free(whole_path);
	if (evb)
		evbuffer_free(evb);
}
