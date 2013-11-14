#include "lua_eval.h"

#include "lua.h"
#include "lauxlib.h"
#include "luajit.h"

#include "glib_defines.h"

G_DEFINE_QUARK (hammy-lua-eval-error, hammy_lua_eval_error)
#define E_DOMAIN hammy_lua_eval_error_quark()
#define EHLUA ECANCELED

struct hammy_lua_eval_priv
{
	lua_State *L;
	GString *entry_point;
	gboolean sandbox;

};

typedef struct hammy_lua_eval_priv *hammy_lua_eval_t;

static gboolean
hammy_lua_eval_load (hammy_lua_eval_t self, GString *code, _H_AERR)
{
	FUNC_BEGIN();

	if (0 != luaL_loadbuffer (self->L, code->str, code->len, "hammy_preload"))
	{
		g_set_error (&lerr, E_DOMAIN, EHLUA, "luaL_loadbuffer: %s",
					lua_tostring (self->L, -1));
		lua_pop (self->L, 1);
		GOTO_END;
	}

	if (0 != lua_pcall (self->L, 0, 0, 0))
	{
		g_set_error (&lerr, E_DOMAIN, EHLUA, "lua_pcall: %s",
					lua_tostring (self->L, -1));
		lua_pop (self->L, 1);
		GOTO_END;
	}

	FUNC_END();
}

gpointer
hammy_lua_eval_new (struct hammy_lua_eval_cfg *cfg, GError **error)
{
	GError *lerr = NULL;
	hammy_lua_eval_t self;
	GSList *code = NULL;

	g_assert (cfg);
	g_assert (cfg->entry_point);
	g_assert (cfg->preload_code);

	self = g_new0 (struct hammy_lua_eval_priv, 1);

	self->entry_point = cfg->entry_point;
	self->sandbox = cfg->sandbox;

	self->L = lua_open ();

	// stdlib
	luaL_openlibs (self->L);

	// cmsgpack
	if (1 != luaopen_cmsgpack (self->L))
	{
		g_set_error (&lerr, E_DOMAIN, EHLUA, "luaopen_cmsgpack: %s",
					lua_tostring (self->L, -1));
		lua_pop (self->L, 1);
		GOTO_END;
	}

	// luajit
	if (1 != luaJIT_setmode (self->L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON))
	{
		g_set_error (&lerr, E_DOMAIN, ECANCELED, "luaJIT_setmode");
		GOTO_END;
	}

	// preload_code
	for (code = cfg->preload_code; code; code = code->next)
	{
		H_TRY (hammy_lua_eval_load (self, code->data, ERR_RETURN));
	}

END:
	if (!lerr)
	{
		return (gpointer)self;
	}
	else
	{
		hammy_lua_eval_free ((gpointer)self);
		return NULL;
	}
}

gboolean
hammy_lua_eval_eval (gpointer priv, GByteArray *data, GError **error)
{
	FUNC_BEGIN();
	hammy_lua_eval_t self = (hammy_lua_eval_t)priv;
	size_t len = 0;

	// Unpack request
	lua_getglobal(self->L, "cmsgpack");
	lua_getfield(self->L, -1, "unpack");
	lua_remove(self->L, -2); // cmsgpack
	lua_pushlstring (self->L, data->data, data->len);
	g_free (data->data);
	data->len = 0;
	if (0 != lua_pcall(self->L, 1, 1, 0))
	{
		g_set_error (&lerr, E_DOMAIN, EHLUA,
					 "lua_pcall [cmsgpack.unpack] error: %s",
					 lua_tostring (self->L, -1));
		lua_pop (self->L, 1);
		GOTO_END;
	}
	lua_setglobal (self->L, "__request");

	// Call function...
	lua_getglobal(self->L, self->entry_point->str);
	if (!lua_isfunction(self->L, -1))
	{
		g_set_error (&lerr, E_DOMAIN, EHLUA,
					 "'%s' is not a function",
					 self->entry_point->str);
		GOTO_END;
	}
	if (0 != lua_pcall (self->L, 0, 0, 0))
	{
		g_set_error (&lerr, E_DOMAIN, EHLUA, "lua_pcall [%s]: %s",
					 self->entry_point->str, lua_tostring (self->L, -1));
		lua_pop (self->L, 1);
		GOTO_END;
	}

	// Pack response
	lua_getglobal(self->L, "cmsgpack");
	lua_getfield(self->L, -1, "pack");
	lua_remove(self->L, -2); // cmsgpack
	lua_getglobal(self->L, "__response");
	if (0 != lua_pcall(self->L, 1, 1, 0))
	{
		g_set_error (&lerr, E_DOMAIN, EHLUA,
					 "lua_pcall [cmsgpack.pack] error: %s",
					 lua_tostring (self->L, -1));
		lua_pop (self->L, 1);
		GOTO_END;
	}

	data->data = (gpointer)lua_tolstring(self->L, -1, &len);
	data->len = len;
	/*
	 * Because Lua has garbage collection, there is no guarantee that the
	 * pointer returned by lua_tolstring will be valid after the corresponding
	 * value is removed from the stack.
	 */
	data->data = g_memdup(data->data, data->len);

	FUNC_END();
}

void
hammy_lua_eval_free (gpointer priv)
{
	hammy_lua_eval_t self = (hammy_lua_eval_t)priv;

	g_assert (self);

	if (self->L)
	{
		lua_close (self->L);
	}

	g_free (self);
}
