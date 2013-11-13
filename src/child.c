#include "child.h"

#include "reader.h"
#include "writer.h"

#include "glib_defines.h"

G_DEFINE_QUARK (hammy-child-error, hammy_child_error)
#define E_DOMAIN hammy_worker_child_quark()

struct hammy_child_priv {
	struct ev_loop *loop;
	int in_socket;
	int out_socket;

	hammy_reader_t reader;
	hammy_writer_t writer;

	struct hammy_eval *eval;
};

static void
hammy_child_writer_cb (gpointer priv, GError *error)
{
	if (error)
	{
		g_error ("Fatal child error [writer]: %s", error->message);
	}
};

static void
hammy_child_reader_cb (gpointer priv, GByteArray *data, GError *error)
{
	GError *lerr = NULL;
	hammy_child_t self = (hammy_child_t)priv;

	if (error)
	{
		g_error ("Fatal child error [reader]: %s", error->message);
	}

	if ((*self->eval->eval) (self->eval->priv, data, &lerr))
	{
		if (!hammy_writer_write (self->writer, data, &lerr))
		{
			g_assert (lerr);
			g_error ("Fatal child error [writer]: %s", lerr->message);
		}
	}
	else
	{
		g_assert (lerr);
		g_error ("Fatal child error [reader]: %s", lerr->message);
	}
};

hammy_child_t
hammy_child_new(struct hammy_child_cfg *cfg, GError **error)
{
	GError *lerr = NULL;

	hammy_child_t self = g_new0 (struct hammy_child_priv, 1);

	g_assert (cfg);
	g_assert (cfg->loop);
	g_assert (cfg->eval);

	self->loop = cfg->loop;
	self->in_socket = cfg->in_socket;
	self->out_socket = cfg->out_socket;
	self->eval = cfg->eval;

	struct hammy_reader_cfg r_cfg;
	r_cfg.fd = self->in_socket;
	r_cfg.loop = self->loop;
	r_cfg.priv = self;
	r_cfg.callback = NULL; // FIXME
	H_TRY (self->reader = hammy_reader_new (&r_cfg, ERR_RETURN));

	struct hammy_writer_cfg w_cfg;
	w_cfg.fd = self->out_socket;
	w_cfg.loop = self->loop;
	w_cfg.priv = self;
	w_cfg.callback = &hammy_child_writer_cb;
	H_TRY (self->writer = hammy_writer_new (&w_cfg, ERR_RETURN));

END:
	if (lerr != NULL)
	{
		if (self->reader)
		{
			hammy_reader_free (self->reader);
		}
		if (self->writer)
		{
			hammy_writer_free (self->writer);
		}
		g_free (self);
		return NULL;
	}
	return self;
}

gboolean
hammy_child_run (hammy_child_t self, GError **error)
{
	FUNC_BEGIN()

	ev_run (self->loop, 0);

	FUNC_END()
}

void
hammy_child_free (hammy_child_t self)
{
	g_free(self);
}
