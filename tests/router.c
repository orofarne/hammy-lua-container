#include "router.h"

#include "main.h"

#include "glib_defines.h"

#include <string.h>
#include <errno.h>

// Network Stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

int initRouterSuite()
{
	return 0;
}

int cleanRouterSuite()
{
	return 0;
}

static gboolean
h_router_eval_func (gpointer priv, GByteArray *data, GError **error)
{
	FUNC_BEGIN()

	// DO NOTHING

	FUNC_END()
}

static gpointer
h_router_thread_func (gpointer data)
{
	GError *err = NULL;
	gboolean rc;
	hammy_router_t r = (hammy_router_t)data;

	rc = hammy_router_run (r, &err);
	g_assert ((rc ? 1 : 0) == (err ? 0 : 1));

	return err;
}

void testRouterTest1()
{
	GError *error = NULL;
	struct hammy_router_cfg cfg;
	struct hammy_eval eval_cfg;
	GThread *th = NULL;
	int sock, msgsock, rval;
	struct sockaddr_un server;

	char buf1[] = {'\xa5', 'H', 'e', 'l', 'l', 'o'};

	memset (&cfg, sizeof(cfg), 0);
	cfg.sock_path = g_strdup ("/tmp/hammy_worker_test.socket");
	cfg.sock_backlog = 100;
	cfg.max_workers = 1;

	eval_cfg.priv = NULL;
	eval_cfg.eval = &h_router_eval_func;
	cfg.eval = &eval_cfg;

	hammy_router_t r = hammy_router_new (&cfg, &error);
	CU_ASSERT_PTR_NOT_NULL (r);
	CU_ASSERT_PTR_NULL (error);

	th = g_thread_new ("router_thread", h_router_thread_func, r);
	g_usleep (1000);

	sock = socket (AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
		g_error ("opening stream socket");
	server.sun_family = AF_UNIX;
	strcpy(server.sun_path, cfg.sock_path);
	if (connect (sock, (struct sockaddr *) &server, sizeof (struct sockaddr_un)) != 0)
	{
		close (sock);
		g_error ("connecting stream socket: %s", strerror (errno));
	}

	// TODO
	CU_ASSERT_EQUAL (
			write (sock, buf1, sizeof (buf1)),
			sizeof (buf1)
		)

	close (sock);

	g_usleep (100000); // FIXME

	CU_ASSERT_EQUAL (hammy_router_stop (r, &error), TRUE);
	CU_ASSERT_PTR_NULL (error);

	// ======= bad
	sock = socket (AF_UNIX, SOCK_STREAM, 0);
	if (sock < 0)
		g_error ("opening stream socket");
	if (connect (sock, (struct sockaddr *) &server, sizeof (struct sockaddr_un)) != 0)
	{
		close (sock);
		g_error ("connecting stream socket: %s", strerror(errno));
	}
	close (sock);
	/// =======

	error = (GError *)g_thread_join (th);
	CU_ASSERT_PTR_NULL (error);

	g_free (cfg.sock_path);
	hammy_router_free (r);
}

int addRouterSuite()
{
	CU_pSuite pSuite = NULL;

	/* add a suite to the registry */
	pSuite = CU_add_suite("RouterSuite", initRouterSuite, cleanRouterSuite);
	if (NULL == pSuite) {
		return CU_get_error();
	}

	/* add the tests to the suite */
	if (
		(NULL == CU_add_test(pSuite, "Test1", testRouterTest1)) ||
		0)
	{
		return CU_get_error();
	}

	return 0;
}
