#include "child.h"

#include "glib_defines.h"

G_DEFINE_QUARK (hammy-child-error, hammy_child_error)
#define E_DOMAIN hammy_worker_child_quark()

struct hammy_child_priv {
	struct ev_loop *loop;
	int in_socket;
	int out_socket;
};

hammy_child_t
hammy_child_new(struct hammy_child_cfg *cfg, GError **error)
{
	hammy_child_t self = g_new0 (struct hammy_child_priv, 1);

	g_assert (cfg != NULL);
	g_assert (cfg->loop != NULL);

	self->loop = cfg->loop;
	self->in_socket = cfg->in_socket;
	self->out_socket = cfg->out_socket;

	return self;
}

gboolean
hammy_child_run (hammy_child_t self, GError **error)
{
	FUNC_BEGIN()

	for(;;)
	{
		// read
		// process
		// write
	}

	FUNC_END()
}

void
hammy_child_free (hammy_child_t self)
{
	g_free(self);
}
