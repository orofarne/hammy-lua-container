#include "router.h"

#include "worker.h"
#include "reader.h"
#include "writer.h"
#include "glib_defines.h"

#include <msglen.h>

#include <string.h>
#include <sys/param.h> // MAXPATHLEN

// Network Stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

// Libev
#include <ev.h>

G_DEFINE_QUARK (hammy-router-error, hammy_router_error)
#define E_DOMAIN hammy_router_error_quark()

struct hammy_router_client
{
	int fd;

	hammy_reader_t reader;
	hammy_writer_t writer;

	struct hammy_router_priv* server;
};

struct hammy_router_task
{
	gpointer data;
	gsize data_size;

	struct hammy_router_client *client;
};

struct hammy_router_priv
{
	struct ev_loop *loop;

	// Config
	gchar* sock_path;
	gint sock_backlog;
	struct hammy_eval *eval;

	// In socket
	struct sockaddr_un socket_un;
	gsize socket_un_len;
	int in_fd;
	ev_io in_io;

	// Clients
	GSList *clients;
	GQueue *tasks;

	// Workers
	guint max_workers;
	GPtrArray *workers;

	// Error
	GError *error;
};

static gboolean
hammy_router_touch_workers (hammy_router_t self, _H_AERR);

static void
hammy_router_client_reader_cb (gpointer priv, GByteArray *data, GError *error)
{
	GError *err = NULL;
	struct hammy_router_task *task;
	struct hammy_router_client *client = (struct hammy_router_client *)priv;

	if (error)
	{
		// FIXME
		g_error ("%s [%d]: %s", __FUNCTION__, __LINE__, error->message);
	}

	task = g_new0 (struct hammy_router_task, 1);
	task->client = client;
	task->data = data->data;
	task->data_size = data->len;

	g_byte_array_free (data, FALSE);

	g_queue_push_tail (client->server->tasks, task);

	if (!hammy_router_touch_workers (client->server, &err))
	{
		// FIXME
		g_error ("%s [%d]: %s", __FUNCTION__, __LINE__, (err ? err->message : "<NULL>"));
	}
}

static void
hammy_router_client_writer_cb (gpointer priv, GError *error)
{

}

static void
hammy_router_client_free (gpointer ptr)
{
	struct hammy_router_client *self = (struct hammy_router_client *)ptr;

	if (self == NULL)
		return;

	if (self->reader)
		hammy_reader_free (self->reader);
	if (self->writer)
		hammy_writer_free (self->writer);

	close (self->fd);
	g_free (self);
}


static struct hammy_router_client *
hammy_router_client_new (hammy_router_t server, int fd, GError **error)
{
	GError *lerr = NULL;
	struct hammy_reader_cfg r_cfg;
	struct hammy_writer_cfg w_cfg;
	struct hammy_router_client *self = g_new0 (struct hammy_router_client, 1);

	self->server = server;
	self->fd = fd;

	r_cfg.fd = self->fd;
	r_cfg.loop = self->server->loop;
	r_cfg.callback = &hammy_router_client_reader_cb;
	self->reader = hammy_reader_new (&r_cfg, &lerr);
	if (!self->reader)
	{
		hammy_router_client_free (self);
		g_propagate_error (error, lerr);
		return NULL;
	}

	w_cfg.fd = self->fd;
	w_cfg.loop = self->server->loop;
	w_cfg.callback = &hammy_router_client_writer_cb;
	self->writer = hammy_writer_new (&w_cfg, &lerr);
	if (!self->writer)
	{
		hammy_router_client_free (self);
		g_propagate_error (error, lerr);
		return NULL;
	}

	return self;
}

static void
hammy_router_task_free (gpointer ptr)
{
	struct hammy_router_task *task = (struct hammy_router_task *)ptr;

	if (task->data != NULL)
		g_free (task->data);
	g_free (task);
}

// Simply adds O_NONBLOCK to the file descriptor of choice
static gboolean
hammy_router_setnonblock(int fd, _H_AERR)
{
	FUNC_BEGIN()

	int flags;

	flags = fcntl (fd, F_GETFL);
	flags |= O_NONBLOCK;
	if (fcntl (fd, F_SETFL, flags) < 0)
		ERRNO_ERR ("fcntl")

	FUNC_END()
}

// worker done
static void
hammy_router_worker_cb (gpointer private, gpointer data, gsize data_size, GError *error)
{
	GError *lerr = NULL;
	GByteArray *buf = NULL;

	struct hammy_router_task *task = (struct hammy_router_task *)private;
	struct hammy_router_client *client = task->client;

	hammy_router_task_free (task);

	if (error)
	{
		g_propagate_error (&lerr, error);
		GOTO_END;
	}

	buf = g_byte_array_new_take (data, data_size);

	H_TRY (hammy_writer_write (client->writer, buf, ERR_RETURN));

END:
	if (lerr)
	{
		if (buf)
			 g_byte_array_free (buf, TRUE);

		g_error ("FIXME [%s:%d]: %s", __FILE__, __LINE__, lerr->message); // FIXME
	}
}

// We have a new task for worker
static gboolean
hammy_router_touch_worker (hammy_router_t self, struct hammy_router_task *task, _H_AERR)
{
	GError *lerr = NULL;
	guint i;

	for (i = 0; i < self->workers->len; ++i)
	{
		hammy_worker_t worker = (hammy_worker_t)self->workers->pdata[i];
		if (!hammy_worker_is_busy (worker))
		{
			if (!hammy_worker_task (worker, task->data, task->data_size, &hammy_router_worker_cb, task, ERR_RETURN))
				break;

			return TRUE;
		}
	}

	if (lerr != NULL)
		g_propagate_error (error, lerr);
	return FALSE;
}

static gboolean
hammy_router_touch_workers (hammy_router_t self, _H_AERR)
{
	FUNC_BEGIN()

	struct hammy_router_task *task;

	for (;;)
	{
		task = (struct hammy_router_task *)g_queue_pop_head (self->tasks);
		if (task == NULL)
			break;

		if (!hammy_router_touch_worker (self, task, ERR_RETURN))
		{
			H_ASSERT_ERROR

			if (self->workers->len < self->max_workers)
			{
				// Create new worker
				struct hammy_worker_cfg w_cfg;
				hammy_worker_t w;

				w_cfg.loop = self->loop;
				w_cfg.eval = self->eval;

				w = hammy_worker_new (&w_cfg, ERR_RETURN);
				g_ptr_array_add (self->workers, w);
				// Return task to queue
				g_queue_push_head (self->tasks, task);
				continue;
			}
			else
			{
				// No free workers
				// Return task to queue
				g_queue_push_head (self->tasks, task);
				break;
			}
		}
	}

	FUNC_END()
}

// This callback is called when data is readable on the unix socket.
static void
hammy_router_accept_cb (struct ev_loop *loop, ev_io *w, int revents)
{
	hammy_router_t self = (hammy_router_t)w->data;
	struct hammy_router_client *client = NULL;
	GError *lerr = NULL;
	int fd;

	g_assert (revents & EV_READ);

	fd = accept (self->in_fd, NULL, NULL);
	if (fd < 0)
	{
		E_SET_ERRNO (&self->error, "accept");
		ev_loop_destroy (EV_DEFAULT_UC); // ???
		return; // break;
	}

	hammy_router_setnonblock (fd, &lerr);
	if (lerr != NULL)
	{
		close (fd);
		g_propagate_error (&self->error, lerr);
		ev_loop_destroy (EV_DEFAULT_UC); // ???
		return; // break;
	}

	client = hammy_router_client_new (self, fd, ERR_RETURN);
	if (!client)
	{
		close (fd);
		g_propagate_error (&self->error, lerr);
		return;
	}

	self->clients = g_slist_prepend (self->clients, client);
}

static gboolean
hammy_router_unix_socket_init (hammy_router_t self, _H_AERR)
{
	FUNC_BEGIN()
	int fd;

	if (unlink (self->sock_path) < 0 && errno != ENOENT)
		ERRNO_ERR ("unlink")

	// Setup a unix socket listener.
	fd = socket (AF_UNIX, SOCK_STREAM, 0);
	if (-1 == fd)
		ERRNO_ERR ("filed to create socket")

	// Set it non-blocking
	H_TRY (hammy_router_setnonblock (fd, ERR_RETURN))

	g_assert (strlen (self->sock_path) <=  MAXPATHLEN);

	// Set it as unix socket
	self->socket_un.sun_family = AF_UNIX;
	strcpy (self->socket_un.sun_path, self->sock_path);

	self->socket_un_len = sizeof (self->socket_un.sun_family) + strlen (self->socket_un.sun_path);
	self->in_fd = fd;

	FUNC_END()
}

static gboolean
hammy_router_listen (hammy_router_t self, _H_AERR)
{
	FUNC_BEGIN()

	if (bind (self->in_fd, (struct sockaddr*)&self->socket_un, self->socket_un_len) < 0)
		ERRNO_ERR ("bind")

	if (listen (self->in_fd, self->sock_backlog) < 0)
		ERRNO_ERR ("listen")

	// Get notified whenever the socket is ready to read
	self->in_io.data = self;
	ev_io_init (&self->in_io, hammy_router_accept_cb, self->in_fd, EV_READ);
	ev_io_start (self->loop, &self->in_io);
	ev_run (self->loop, 0);

	FUNC_END()
}

hammy_router_t
hammy_router_new (struct hammy_router_cfg *cfg, GError **error)
{
	hammy_router_t self;

	g_assert (cfg->sock_path);
	g_assert (cfg->eval);

	self = g_new0 (struct hammy_router_priv, 1);

	self->sock_path = g_strdup (cfg->sock_path);
	self->sock_backlog = cfg->sock_backlog;

	self->clients = g_slist_alloc ();
	self->tasks = g_queue_new ();

	self->workers = g_ptr_array_new ();
	g_ptr_array_set_free_func (self->workers, hammy_worker_free_ptr);
	self->max_workers = cfg->max_workers;

	self->loop = ev_default_loop (EVFLAG_AUTO);

	self->eval = cfg->eval;

	return self;
}

void
hammy_router_free (hammy_router_t self)
{
	if (self->sock_path != NULL)
		g_free (self->sock_path);
	if (self->tasks != NULL)
		g_queue_free_full (self->tasks, hammy_router_task_free);
	if (self->clients != NULL)
		g_slist_free_full (self->clients, hammy_router_client_free);
	if (self->workers != NULL)
		g_ptr_array_free (self->workers, TRUE);
	if (self->loop != NULL)
		ev_loop_destroy (self->loop);
	g_free (self);
}

gboolean
hammy_router_run (hammy_router_t self, GError **error)
{
	FUNC_BEGIN()

	H_TRY (hammy_router_unix_socket_init (self, ERR_RETURN));

	H_TRY (hammy_router_listen (self, ERR_RETURN));

	if (self->error != NULL)
	{
		g_propagate_error (ERR_RETURN, self->error);
		return FALSE;
	}

	FUNC_END()
}

gboolean
hammy_router_stop (hammy_router_t self, GError **error)
{
	FUNC_BEGIN()

	// TODO

	if (self->tasks != NULL)
	{
		g_queue_free_full (self->tasks, hammy_router_task_free);
		self->tasks = NULL;
	}

	if (self->clients != NULL)
	{
		g_slist_free_full (self->clients, hammy_router_client_free);
		self->clients = NULL;
	}

	ev_io_stop (self->loop, &self->in_io);
	ev_break (self->loop, EVBREAK_ALL);

	FUNC_END()
}
