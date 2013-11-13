#include "writer.h"

#include "glib_defines.h"

// Network Stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

G_DEFINE_QUARK (hammy-writer-error, hammy_writer_error)
#define E_DOMAIN hammy_writer_error_quark()


struct hammy_writer_priv
{
	int fd;
	struct ev_loop *loop;
	ev_io io;

	GByteArray *current_buffer;
	goffset current_offset;
	GQueue *buffers;

	gpointer priv;
	void (*callback)(gpointer priv, GError *error);
};

static void
hammy_writer_writable_cb (struct ev_loop *loop, ev_io *w, int revents)
{
	hammy_writer_t self = (hammy_writer_t)w->data;
	ssize_t n;
	GError *err = NULL;

	g_assert (revents & EV_WRITE);

	if (!self->current_buffer)
	{
		if (g_queue_is_empty (self->buffers))
		{
			ev_io_stop (self->loop, &self->io);
			return;
		}
		self->current_buffer = g_queue_pop_head (self->buffers);
		self->current_offset = 0;
	}

	n = send (self->fd, self->current_buffer->data + self->current_offset,
						self->current_buffer->len - self->current_offset,
						MSG_DONTWAIT | MSG_NOSIGNAL);

	if (n < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return; // Wait...
		}
		else
		{
			ev_io_stop (self->loop, &self->io);
			E_SET_ERRNO (&err, "write");
			(*self->callback) (self->priv, err);
			return;
		}
	}

	if (n == self->current_buffer->len - self->current_offset)
	{
		g_byte_array_free (self->current_buffer, TRUE);
		self->current_buffer = NULL;
		self->current_offset = 0;
		hammy_writer_writable_cb (loop, w, revents);
		return;
	}

	self->current_offset += n;
}

hammy_writer_t
hammy_writer_new (struct hammy_writer_cfg *cfg, GError **error)
{
	hammy_writer_t self = NULL;

	g_assert (cfg);
	g_assert (cfg->loop);
	g_assert (cfg->callback);

	self = g_new0 (struct hammy_writer_priv, 1);

	self->io.data = self;
	self->fd = cfg->fd;
	self->loop = cfg->loop;
	self->priv = cfg->priv;
	self->callback = cfg->callback;

	self->buffers = g_queue_new ();

	// Start io
	ev_io_init (&self->io, &hammy_writer_writable_cb, self->fd, EV_WRITE);

	return self;
}

static void
hammy_writer_g_byte_array_free (gpointer ptr)
{
	if (ptr)
	{
		g_byte_array_free ((GByteArray *)ptr, TRUE);
	}
}

void
hammy_writer_free (hammy_writer_t self)
{
	ev_io_stop (self->loop, &self->io);
	if (self->current_buffer)
		g_byte_array_free (self->current_buffer, TRUE);
	g_queue_free_full (self->buffers, hammy_writer_g_byte_array_free);
	g_free (self);
}

gboolean
hammy_writer_write (hammy_writer_t self, GByteArray *buf, _H_AERR)
{
	FUNC_BEGIN();

	g_queue_push_tail (self->buffers, buf);

	if (!self->current_buffer)
	{
		ev_io_start (self->loop, &self->io);
	}

	FUNC_END();
}
