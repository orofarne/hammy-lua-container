#include "reader.h"

#include "main.h"

// Network Stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

static int pipefd[2];
static struct ev_loop *loop;
static size_t bytes;

static int initReaderSuite()
{
	int i, flags;
	g_assert (pipe (pipefd) == 0);
	for(i = 0; i < 2; ++i)
	{
		register int fd = pipefd[i];
		flags = fcntl (fd, F_GETFL);
		flags |= O_NONBLOCK;
		g_assert (fcntl (fd, F_SETFL, flags) == 0);
	}
	loop = ev_default_loop (EVFLAG_AUTO);
	bytes = 0;
	return 0;
}

static int cleanReaderSuite()
{
	int i;
	for (i = 0; i < 2; ++i)
	{
		close (pipefd[i]);
	}
	ev_loop_destroy (loop);
	return 0;
}

static gpointer
h_reader_test_thread_func (gpointer data)
{
	ssize_t *n = g_new0 (ssize_t, 1);
	GByteArray *buf = (GByteArray *)data;

	*n = write (pipefd[1], buf->data, buf->len);

	return n;
}

static void testReaderHelloCallback(gpointer priv, GByteArray *data, GError *error)
{
	CU_ASSERT_PTR_NULL (error);
	CU_ASSERT_PTR_NOT_NULL_FATAL (data);

	bytes += data->len;
	if (data->len == 6)
	{
		CU_ASSERT_NSTRING_EQUAL (data->data, "\xa5Hello", data->len);
	}

	if (data->len == 4)
	{
		CU_ASSERT_NSTRING_EQUAL (data->data, "\xa3Hi!", data->len);
	}

	if (bytes >= 10)
	{
		ev_break (loop, EVBREAK_ALL);
	}
}

static void testReaderHello()
{
	GError *error = NULL;
	GThread *th = NULL;
	char buf1[] = {'\xa5', 'H', 'e', 'l', 'l', 'o', '\xa3', 'H', 'i', '!'};
	GByteArray buf;
	struct hammy_reader_cfg cfg;
	hammy_reader_t r;
	ssize_t *n;

	buf.data = (gpointer)buf1;
	buf.len = 10;
	th = g_thread_new ("reader_test_thread", h_reader_test_thread_func, &buf);

	cfg.fd = pipefd[0];
	cfg.loop = loop;
	cfg.callback = &testReaderHelloCallback;

	r = hammy_reader_new(&cfg, &error);
	CU_ASSERT_PTR_NULL_FATAL (error);

	ev_run (loop, 0);
	n = g_thread_join (th);

	CU_ASSERT_EQUAL (*n, buf.len);
	g_free(n);

	CU_ASSERT_EQUAL (bytes, buf.len);
}

int addReaderSuite()
{
	CU_pSuite pSuite = NULL;

	/* add a suite to the registry */
	pSuite = CU_add_suite("ReaderSuite", initReaderSuite, cleanReaderSuite);
	if (NULL == pSuite) {
		return CU_get_error();
	}

	/* add the tests to the suite */
	if (
		(NULL == CU_add_test(pSuite, "Hello", testReaderHello)) ||
		0)
	{
		return CU_get_error();
	}

	return 0;
}
