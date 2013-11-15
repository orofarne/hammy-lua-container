#include "router.h"
#include "lua_eval.h"

#include <glib.h>
#include <glib/gstdio.h>
#include "glib_defines.h"

#include <string.h>

G_DEFINE_QUARK(hammy-main-error, hammy_main_error)
#define E_DOMAIN hammy_main_error_quark()

#define CFG_SECTION "lua"

static gchar *cfg_file = NULL;

static GOptionEntry entries[] =
{
	{ "config", 'c', 0, G_OPTION_ARG_STRING, &cfg_file, "Config file path", "FILE" },
	{ NULL }
};

static void freeGString (gpointer ptr)
{
	g_string_free (ptr, TRUE);
}

static GSList *
load_code (const gchar *path, _H_AERR)
{
	GError *lerr = NULL;
	GDir *dir = NULL;
	GSList *code = NULL;

	dir = g_dir_open (path, 0, ERR_RETURN);
	H_ASSERT_ERROR

	const gchar *f;
	while (f = g_dir_read_name (dir))
	{
		size_t len = strlen (f);
		if (len < 5)
			continue;
		if (0 != strncmp (".lua", (f + len - 4), 4))
			continue;
		GByteArray *c = g_byte_array_new ();
		gsize s;

		GString *fname = g_string_new (NULL);
		g_string_append_printf (fname, "%s/%s", path, f);
		if (!g_file_get_contents (fname->str, (gchar **)&c->data, &s, ERR_RETURN))
		{
			g_string_free (fname, TRUE);
			g_byte_array_free (c, TRUE);
			GOTO_END;
		}
		g_string_free (fname, TRUE);
		c->len = s;
		code = g_slist_append (code, c);
	}

	if (code == NULL)
	{
		g_set_error (&lerr, E_DOMAIN, ENOENT, "Nothing to execute");
		GOTO_END;
	}

END:
	if (dir != NULL)
			g_dir_close (dir);
	if (lerr != NULL)
	{
		if (code != NULL)
			g_slist_free_full (code, &freeGString);
		code = NULL;
		g_propagate_error (error, lerr);
	}
	return code;
}

static gboolean
start_router (GKeyFile *cfg_kv, _H_AERR)
{
	FUNC_BEGIN()
	hammy_router_t router = NULL;
	struct hammy_router_cfg cfg;
	struct hammy_eval eval_cfg;
	struct hammy_lua_eval_cfg le_cfg;
	gchar *code_path = NULL;

	memset (&cfg, 0, sizeof (cfg));
	memset (&le_cfg, 0, sizeof (le_cfg));
	memset (&eval_cfg, 0, sizeof (eval_cfg));

	cfg.sock_path = g_key_file_get_string (cfg_kv, CFG_SECTION, "sock_path", ERR_RETURN);
	H_ASSERT_ERROR
	cfg.sock_backlog = g_key_file_get_uint64 (cfg_kv, CFG_SECTION, "sock_backlog", ERR_RETURN);
	H_ASSERT_ERROR
	cfg.max_workers = g_key_file_get_uint64 (cfg_kv, CFG_SECTION, "max_workers", ERR_RETURN);
	H_ASSERT_ERROR
	code_path = g_key_file_get_string (cfg_kv, CFG_SECTION, "code_path", ERR_RETURN);
	H_ASSERT_ERROR
	le_cfg.entry_point = g_key_file_get_string (cfg_kv, CFG_SECTION, "entry_point", ERR_RETURN);
	H_ASSERT_ERROR

	le_cfg.preload_code = load_code (code_path, ERR_RETURN);
	H_ASSERT_ERROR

	eval_cfg.priv = hammy_lua_eval_new (&le_cfg, ERR_RETURN);
	if (!eval_cfg.priv)
		GOTO_END;
	eval_cfg.eval = &hammy_lua_eval_eval;
	cfg.eval = &eval_cfg;

	router = hammy_router_new (&cfg, ERR_RETURN);
	if (router == NULL)
		GOTO_END

	if (!hammy_router_run (router, ERR_RETURN))
		GOTO_END

	FUNC_END(
		if (cfg.sock_path != NULL)
			g_free (cfg.sock_path);
		if (router != NULL)
			hammy_router_free (router);
		if (eval_cfg.priv != NULL)
			hammy_lua_eval_free (eval_cfg.priv);
		if (code_path != NULL)
			g_free (code_path);
		if (le_cfg.preload_code != NULL)
			g_slist_free_full (le_cfg.preload_code, &freeGString);
		if (le_cfg.entry_point != NULL)
			g_free (le_cfg.entry_point);
	)
}

int
main (int argc, char *argv[])
{
	GError *error = NULL;
	GOptionContext *context;
	GKeyFile *cfg_kv = NULL;
	gint rc = 1;

	context = g_option_context_new (NULL);
	g_option_context_add_main_entries (context, entries, NULL);
	if (!g_option_context_parse (context, &argc, &argv, &error))
	{
		g_print ("option parsing failed: %s\n", error->message);
		goto END;
	}

	if (cfg_file == NULL) {
		gchar *help_str = g_option_context_get_help (context, TRUE, NULL);
		g_print ("%s", help_str);
		g_free (help_str);
		goto END;
	}

	cfg_kv = g_key_file_new ();
	if (!g_key_file_load_from_file (cfg_kv, cfg_file, 0, &error))
	{
		g_print ("failed to read config: %s\n", error->message);
		goto END;
	}

	if (!start_router(cfg_kv, &error))
	{
		g_print ("failed to start router: %s\n", error->message);
		goto END;
	}

	rc = 0;
END:
	if (error != NULL)
		g_error_free (error);
	if (context != NULL)
		g_option_context_free (context);
	if (cfg_kv != NULL)
		g_key_file_free (cfg_kv);
	return rc;
}

