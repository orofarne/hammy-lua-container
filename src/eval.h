#pragma once

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

struct hammy_eval
{
	gpointer priv;
	gboolean (*eval)(gpointer priv, GByteArray *data, GError **error);
};

#ifdef __cplusplus
}
#endif
