#pragma once

#include <glib.h>
// Libev
#include <ev.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hammy_writer_cfg
{
	int fd;
	struct ev_loop *loop;

	void (*callback)(GError *error);
};

struct hammy_writer_priv;
typedef struct hammy_writer_priv *hammy_writer_t;

hammy_writer_t
hammy_writer_new (struct hammy_writer_cfg *cfg, GError **error);

void
hammy_writer_free (hammy_writer_t self);

gboolean
hammy_writer_write (hammy_writer_t self, GByteArray *buf, GError **error);

#ifdef __cplusplus
}
#endif
