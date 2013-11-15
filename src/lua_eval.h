#pragma once

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hammy_lua_eval_cfg
{
	GSList *preload_code; // List of GString
	gchar *entry_point; // Name of eval function in lua code
};

gpointer
hammy_lua_eval_new (struct hammy_lua_eval_cfg *cfg, GError **error);

gboolean
hammy_lua_eval_eval (gpointer priv, GByteArray *data, GError **error);

void
hammy_lua_eval_free (gpointer priv);

#ifdef __cplusplus
}
#endif
