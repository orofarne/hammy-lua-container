#include "reader.h"
#include "writer.h"

#include "main.h"

// Network Stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

#define N_T_ITER 50

static int pipefd[2];
static struct ev_loop *loop;
int res_i;

static int initWriterSuite()
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
	res_i = 0;
	return 0;
}

static int cleanWriterSuite()
{
	int i;
	for (i = 0; i < 2; ++i)
	{
		close (pipefd[i]);
	}
	ev_loop_destroy (loop);
	return 0;
}


static void testWriterHelloCallback(GError *error)
{
	g_assert (error);
}

static void testWriterHelloReaderCallback(GByteArray *data, GError *error)
{
	GString *msg = g_string_new (NULL);
	g_string_append_printf (msg, "\xa8Hello%3d", res_i);

	CU_ASSERT_NSTRING_EQUAL (data->data, msg->str, 9);

	g_string_free(msg, TRUE);

	if (++res_i >= N_T_ITER)
	{
		ev_break (loop, EVBREAK_ALL);
	}
}

static void testWriterHello()
{
	GError *error = NULL;
	struct hammy_writer_cfg w_cfg;
	hammy_writer_t w;
	struct hammy_reader_cfg r_cfg;
	hammy_reader_t r;
	int i;

	w_cfg.fd = pipefd[1];
	w_cfg.loop = loop;
	w_cfg.callback = &testWriterHelloCallback;

	w = hammy_writer_new(&w_cfg, &error);
	CU_ASSERT_PTR_NULL_FATAL (error);

	r_cfg.fd = pipefd[0];
	r_cfg.loop = loop;
	r_cfg.callback = &testWriterHelloReaderCallback;

	r = hammy_reader_new(&r_cfg, &error);
	CU_ASSERT_PTR_NULL_FATAL (error);

	for (i = 0; i < N_T_ITER; ++i)
	{
		GString *msg = g_string_new (NULL);
		GByteArray *buf = g_byte_array_new ();

		g_string_append_printf (msg, "\xa8Hello%3d", i);
		buf->data = msg->str;
		buf->len = 9;

		hammy_writer_write (w, buf, &error);

		CU_ASSERT_PTR_NULL_FATAL (error);

		g_string_free(msg, FALSE);
	}

	ev_run (loop, 0);

	CU_ASSERT_EQUAL (res_i, N_T_ITER);
}

int addWriterSuite()
{
	CU_pSuite pSuite = NULL;

	/* add a suite to the registry */
	pSuite = CU_add_suite("WriterSuite", initWriterSuite, cleanWriterSuite);
	if (NULL == pSuite) {
		return CU_get_error();
	}

	/* add the tests to the suite */
	if (
		(NULL == CU_add_test(pSuite, "Hello", testWriterHello)) ||
		0)
	{
		return CU_get_error();
	}

	return 0;
}
