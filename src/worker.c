#include "worker.h"

#include "reader.h"
#include "writer.h"
#include "child.h"

#include "glib_defines.h"

#include <msglen.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>

G_DEFINE_QUARK (hammy-worker-error, hammy_worker_error)
#define E_DOMAIN hammy_worker_error_quark()

struct hammy_worker_priv
{
	gboolean busy;
	pid_t pid;
	int to_pfd[2];
	int from_pfd[2];
	struct ev_loop *loop;

	hammy_reader_t reader;
	hammy_writer_t writer;

	hammy_worker_task_cb callback;
	gpointer callback_private;

	struct hammy_eval *eval;
};

static gboolean
hammy_worker_child (hammy_worker_t self, _H_AERR)
{
	FUNC_BEGIN()

	hammy_child_t ch = NULL;
	struct hammy_child_cfg child_cfg;

	child_cfg.loop = self->loop;
	child_cfg.eval = self->eval;
	child_cfg.in_socket = self->to_pfd[0];
	child_cfg.out_socket = self->from_pfd[1];

	ch = hammy_child_new (&child_cfg, ERR_RETURN);
	if (!ch)
		H_ASSERT_ERROR

	if (!hammy_child_run (ch, ERR_RETURN))
		H_ASSERT_ERROR

	FUNC_END(hammy_child_free(ch))
}

static gboolean
hammy_worker_fork (hammy_worker_t self, _H_AERR)
{
	FUNC_BEGIN()

	pid_t pid = fork();

	if (pid < 0)
		ERRNO_ERR ("fork");

	if (pid == 0)
	{
		// Child
		ev_loop_fork (self->loop);
		ev_loop_destroy (self->loop);
		self->loop = ev_default_loop (EVFLAG_AUTO);

		if (!hammy_worker_child (self, ERR_RETURN))
		{
			g_assert (lerr != NULL);
			g_error ("[WORKER] Child error: %s", lerr->message);
		}

		exit (0);
	}
	else
	{
		// Parent
		self->pid = pid;
	}

	FUNC_END()
}

// Propagate answer or error from worker
static void
hammy_worker_reader_cb (gpointer priv, GByteArray *data, GError *error)
{
	hammy_worker_t self = (hammy_worker_t)priv;

	if (error)
	{
		if (data)
		{
			g_byte_array_free (data, TRUE);
		}
		(*self->callback)(self->callback_private, NULL, 0, error);
	}
	else
	{
		(*self->callback)(self->callback_private, data->data, data->len, NULL);
		g_byte_array_free (data, FALSE);
	}

	self->callback = NULL;
	self->callback_private = NULL;
	self->busy = FALSE;
}

// Propagate error from worker
static void
hammy_worker_writer_cb (gpointer priv, GError *error)
{
	hammy_worker_t self = (hammy_worker_t)priv;

	g_assert (self->callback);

	(*self->callback)(self->callback_private, NULL, 0, error);
}

hammy_worker_t
hammy_worker_new (struct hammy_worker_cfg *cfg, GError **error)
{
	GError *lerr = NULL;

	struct hammy_worker_priv *self = g_new0 (struct hammy_worker_priv, 1);

	g_assert (cfg);
	g_assert (cfg->loop);
	g_assert (cfg->eval);

	self->loop = cfg->loop;
	self->eval = cfg->eval;

	if (pipe (self->to_pfd) < 0)
		ERRNO_ERR ("pipe (to)");
	if (pipe (self->from_pfd) < 0)
		ERRNO_ERR ("pipe (from)");

	// Set nonblock
	if (fcntl(self->to_pfd[0], F_SETFL, O_NONBLOCK) < 0)
		ERRNO_ERR ("fcntl");
	if (fcntl(self->to_pfd[1], F_SETFL, O_NONBLOCK) < 0)
		ERRNO_ERR ("fcntl");
	if (fcntl(self->from_pfd[0], F_SETFL, O_NONBLOCK) < 0)
		ERRNO_ERR ("fcntl");
	if (fcntl(self->from_pfd[1], F_SETFL, O_NONBLOCK) < 0)
		ERRNO_ERR ("fcntl");

	if (!hammy_worker_fork (self, ERR_RETURN))
		H_ASSERT_ERROR

	struct hammy_reader_cfg r_cfg;
	r_cfg.fd = self->from_pfd[0];
	r_cfg.loop = self->loop;
	r_cfg.priv = self;
	r_cfg.callback = &hammy_worker_reader_cb;
	H_TRY (self->reader = hammy_reader_new (&r_cfg, ERR_RETURN));

	struct hammy_writer_cfg w_cfg;
	w_cfg.fd = self->to_pfd[1];
	w_cfg.loop = self->loop;
	w_cfg.priv = self;
	w_cfg.callback = &hammy_worker_writer_cb;
	H_TRY (self->writer = hammy_writer_new (&w_cfg, ERR_RETURN));


END:
	if (lerr != NULL)
	{
		close (self->to_pfd[0]); close (self->to_pfd[1]);
		close (self->from_pfd[0]); close (self->from_pfd[1]);
		g_free (self);
		g_propagate_error (error, lerr);
		return NULL;
	}
	return self;
}

void
hammy_worker_free (hammy_worker_t self)
{
	close (self->to_pfd[0]); close (self->to_pfd[1]);
	close (self->from_pfd[0]); close (self->from_pfd[1]);
	g_free (self);
}

void
hammy_worker_free_ptr (gpointer ptr)
{
	hammy_worker_free ((hammy_worker_t)ptr);
}

gboolean
hammy_worker_task (hammy_worker_t self, gpointer data, gsize data_size, hammy_worker_task_cb callback, gpointer callback_private, GError **error)
{
	FUNC_BEGIN()

	GByteArray *buf;

	g_assert (!hammy_worker_is_busy (self));
	self->busy = TRUE;

	self->callback = callback;
	self->callback_private = callback_private;

	buf = g_byte_array_new_take (g_memdup (data, data_size), data_size);

	if (!hammy_writer_write (self->writer, buf, ERR_RETURN))
	{
		g_byte_array_free (buf, TRUE);
		GOTO_END;
	}

	FUNC_END()
}

gboolean
hammy_worker_is_busy (hammy_worker_t self)
{
	return self->busy;
}
