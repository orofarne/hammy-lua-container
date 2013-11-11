#include "main.h"

#define HAMMY_ADD_SUITE(x) \
	rc = x(); \
	if (rc) \
	{ \
		return rc; \
	}

int setUpTests()
{
	int rc;

	HAMMY_ADD_SUITE( addReaderSuite );
	HAMMY_ADD_SUITE( addWriterSuite );
	HAMMY_ADD_SUITE( addRouterSuite );

	return 0;
}

int main()
{
	/* initialize the CUnit test registry */
	if (CUE_SUCCESS != CU_initialize_registry())
		return CU_get_error();

	int rc = setUpTests();
	if (rc)
	{
		CU_cleanup_registry();
		return rc;
	}

	/* Run all tests using the CUnit Basic interface */
	CU_basic_set_mode(CU_BRM_VERBOSE);
	CU_basic_run_tests();
	CU_cleanup_registry();
	return CU_get_error();
}
