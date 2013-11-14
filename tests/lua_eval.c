#include "main.h"

#include "lua_eval.h"

static int initLuaEvalSuite ()
{
	return 0;
}

static int cleanLuaEvalSuite ()
{
	return 0;
}

static void freeGString (gpointer ptr)
{
	g_string_free (ptr, TRUE);
}

static void testLuaEvalHello ()
{
	GError *err = NULL;
	GSList *code = NULL;
	GByteArray *d = NULL;
	GString *ep;
	struct hammy_lua_eval_cfg cfg;
	gpointer l = NULL;

	memset (&cfg, 0, sizeof(cfg));
	ep = g_string_new ("echo");
	code = g_slist_prepend (code, g_string_new (
						"function echo()\n"
						"	__response = __request\n"
						"end\n"
						));

	cfg.preload_code = code;
	cfg.entry_point = ep;
	cfg.sandbox = TRUE;
	l = hammy_lua_eval_new(&cfg, &err);
	CU_ASSERT_PTR_NOT_NULL_FATAL (l);
	CU_ASSERT_PTR_NULL_FATAL (err);

	d = g_byte_array_new_take (g_memdup("\xa5Hello", 6), 6);
	hammy_lua_eval_eval (l, d, &err);
	CU_ASSERT_PTR_NULL_FATAL (err);
	CU_ASSERT_PTR_NOT_NULL_FATAL (d->data);
	CU_ASSERT_NSTRING_EQUAL (d->data, "\xa5Hello", 6);

	hammy_lua_eval_free (l);
	g_slist_free_full (code, &freeGString);
	g_string_free (ep, TRUE);
	g_byte_array_free (d, TRUE);
}

int addLuaEvalSuite()
{
	CU_pSuite pSuite = NULL;

	/* add a suite to the registry */
	pSuite = CU_add_suite("LuaEvalSuite", initLuaEvalSuite, cleanLuaEvalSuite);
	if (NULL == pSuite) {
		return CU_get_error();
	}

	/* add the tests to the suite */
	if (
		(NULL == CU_add_test(pSuite, "Hello", testLuaEvalHello)) ||
		0)
	{
		return CU_get_error();
	}

	return 0;
}
