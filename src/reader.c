#include "reader.h"

#include "glib_defines.h"

#include <msglen.h>

// Network Stuff
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>

G_DEFINE_QUARK (hammy-reader-error, hammy_reader_error)
#define E_DOMAIN hammy_reader_error_quark()

#define HAMMY_READER_BUFF_S 1024

struct hammy_reader_priv
{
	int fd;
	struct ev_loop *loop;
	ev_io io;

	gpointer buffer;
	gsize buffer_size;
	gsize buffer_capacity;

	void (*callback)(GByteArray *data, GError *error);
};

// This callback is called when client data is readable on the socket.
static void
hammy_reader_readable_cb (struct ev_loop *loop, ev_io *w, int revents)
{
	hammy_reader_t self = (hammy_reader_t)w->data;
	ssize_t n;
	char *msgpackclen_err = NULL;
	size_t m;
	GByteArray *data = NULL;
	GError *err = NULL;

	n = read (self->fd, self->buffer, self->buffer_capacity);
	if (n < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return; // Wait for data
		}
		else
		{
			E_SET_ERRNO (&err, "read");
		}
	}
	else if (n == 0)
	{
		// Dicconnect
		// keep data NULL
	}
	else
	{
		self->buffer_size += n;

		for(;;) {
			m = msgpackclen_buf_read (self->buffer, self->buffer_size, &msgpackclen_err);
			if (msgpackclen_err != NULL)
			{
				// Fatal error
				g_error ("hammy_msg_buf_read: %s", msgpackclen_err);
			}
			if (m > 0)
			{
				data = g_byte_array_new ();
				data->data = g_memdup (self->buffer, m);
				data->len = m;

				if (m != self->buffer_size)
				{
					memmove (self->buffer, (char *)self->buffer + m, self->buffer_size - m);
				}
				self->buffer_size -= m;
			}
			else
			{
				if (n == self->buffer_capacity)
				{
					self->buffer_capacity *= 2;
					self->buffer = g_realloc (self->buffer, self->buffer_capacity);
				}

				return; // prevent callback call
			}

			(*self->callback)(data, err);
		}
		return;
	}

	(*self->callback)(data, err);
}

hammy_reader_t
hammy_reader_new (struct hammy_reader_cfg *cfg, GError **error)
{
	hammy_reader_t self = NULL;

	g_assert (cfg);
	g_assert (cfg->loop);
	g_assert (cfg->callback);

	self = g_new0 (struct hammy_reader_priv, 1);

	self->io.data = self;
	self->fd = cfg->fd;
	self->loop = cfg->loop;
	self->callback = cfg->callback;

	// Initialize buffer
	self->buffer_size = 0;
	self->buffer_capacity = HAMMY_READER_BUFF_S;
	self->buffer = g_malloc (self->buffer_capacity);

	// Start io
	ev_io_init (&self->io, &hammy_reader_readable_cb, self->fd, EV_READ);
	ev_io_start (self->loop, &self->io);

	return self;
}

void
hammy_reader_free (hammy_reader_t self)
{
	ev_io_stop (self->loop, &self->io);
	if (self->buffer)
		g_free (self->buffer);
	g_free (self);
}
