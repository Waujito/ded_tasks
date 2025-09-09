#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#ifdef USE_GTEST

#include <gtest/gtest.h>

#else /* USE_GTEST */

#include "test_machine.h"

#endif /* USE_GTEST */

#endif /* TEST_CONFIG_H */
