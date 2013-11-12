#pragma once

#include <glib.h>
// Libev
#include <ev.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hammy_reader_cfg
{
	int fd;
	struct ev_loop *loop;

	gpointer priv;
	void (*callback)(gpointer priv, GByteArray *data, GError *error);
};

struct hammy_reader_priv;
typedef struct hammy_reader_priv *hammy_reader_t;

hammy_reader_t
hammy_reader_new (struct hammy_reader_cfg *cfg, GError **error);

void
hammy_reader_free (hammy_reader_t self);

#ifdef __cplusplus
}
#endif
