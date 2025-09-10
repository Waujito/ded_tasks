#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <time.h>

#include "test_config.h"

#include "strfuncs.h"

TEST(StrFuncs, StrLen)
{
	size_t len = my_strlen("abacaba\0");
	ASSERT_EQ((int) len, 7);
}

