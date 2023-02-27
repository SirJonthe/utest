# utest
## Copyright
Copyright Jonathan Karlsson, 2023

github.com/SirJonthe

## About
`utest` is a minimalist unit testing framework for C++.

## Design
`utest` is designed to be as minimalist as possible while still achieving the goal of being a handy tool to do unit testing with. 

## Usage
`utest` provides only a few contact points for the programmer to use. Once a unit tests are correctly defined using the provided library functionality the tests are automatically run when the user calls a single `Run` function.

## Macros
In order to create a unit test, the user needs to make use of two macros; `UTEST_BEGIN(unit_test_name)` and `UTEST_END(unit_test_name, must_pass)`. These macros add boilerplate code which sets up the unit test as well as ensures that the unit test is automatically added to the list of unit tests to be run when calling `Run`. When naming unit tests, use snake case to describe the test as `utest` automatically detects the wording.

## Functions
`Run` runs all user-defined unit tests, assuming they have been created correctly.

## Building
No special adjustments need to be made to build `utest`. Simply include the relevant headers in your code and make sure the headers and source files are available in your compiler search paths. Using `g++` as an example, building is no harder than:

```
g++ code.cpp utest/utest.cpp
```

...where `code.cpp` is an example source file containing the user-defined unit tests as well as the entry point for the program.

## Examples
### Creating a unit test:
Creating a unit test is as easy as using two macros inside an implementation file.

```
#include <iostream>
#include "utest/utest.h"

UTEST_BEGIN(hello_utest)
{
	std::cout << "Hello, utest!" << std::endl;
}
UTEST_END(hello_utest, false)
```

The final boolean parameter in `UTEST_END` will be covered later.

Unit tests should be created inside an implementation file, such as `.c` and `.cpp` files. This reduces pollution and exposing internals, but also allows the user to more easily selectively compile only the tests for pure test builds, which also reduces size of the release targets.

### Asserting success:
Use `UTEST_ASSERT`.

```
#include "utest/utest.h"

UTEST_BEGIN(do_assert)
{
	UTEST_ASSERT(1, ==, 1)
}
UTEST_END(do_assert, false)
```

Note that `UTEST_ASSERT` takes three parameters; the left-hand side value, the comparison operation, and the right-hand side value.

A failed assert automatically kills the execution of the test.

### Preventing further execution on test failure:
The final boolean parameter in `UTEST_END` controls whether execution of the test suite should abort if the current test fails. By setting this to true, the below test will never run the `another_test` test:

```
#include <iostream>
#include "utest/utest.h"

// must_pass will always fail.
UTEST_BEGIN(must_pass)
{
	UTEST_ASSERT(false, ==, true)
}
UTEST_END(must_pass, true)

// This test will never run, since must_pass will always fail.
UTEST_BEGIN(another_test)
{
	std::cout << ":(" << std::endl;
}
UTEST_END(another_test)
```

Tests execute in the order of occurrence in-code and test that must pass will only prevent tests defined within the same file as itself from running.

### Running unit tests:
All tests created using the method as defined above are run using the `Run` function.

```
#include "utest/utest.h"

int main(int, char**)
{
	return utest::Run();
}
```

`Run` returns `1` when some test that was run fails, and `0` otherwise.

### Running only unit tests in specified files:
Tests defined inside specific files can be made to run explicitly while omitting all other tests.

test1.cpp
```
#include "utest/utest.h"

UTEST_BEGIN(test1)
{}
UTEST_END(test1, false)
```

test2.cpp
```
#include "utest/utest.h"

UTEST_BEGIN(test2)
{}
UTEST_END(test2, false)
```

test3.cpp
```
#include "utest/utest.h"

UTEST_BEGIN(test3)
{}
UTEST_END(test3, false)
```

main.cpp
```
#include "utest/utes.h"

int main(int, char**)
{
	const char *tests_to_run[] = {
		"test1.cpp",
		"test3.cpp"
	};
	return utest::Run(tests_to_run, 2);
}
```

The above code will run only tests inside `test1.cpp` and `test3.cpp` despite the tests inside `test2.cpp` also being defined and loaded up inside the `utest` framework.

### Contexts

`utest` does not have direct support for contexts, but can be used to implement them none the less via standard unit tests.

```
#include "utest/utest.h"

// Fixture values.
int *fixture_values = nullptr;
int fixture_value_count = 0;

// Context setup.
UTEST_BEGIN(context_setup)
	fixture_value_count = 0;
	delete [] fixture_values;
	fixture_values = new int[fixture_value_count];
	for (int i = 0; i < fixture_value_count; ++i) {
		fixture_values[i] = i;
	}
UTEST_END(context_setup, true)

// Tests go between the setup and cleanup.
UTEST_BEGIN(use_global_context)
UTEST_END(use_global_context, false)

// Context cleanup
UTEST_BEGIN(context_cleanup)
	delete [] fixture_values;
	fixture_values = nullptr;
	fixture_value_count = 0;
UTEST_END(context_cleanup, true)
```

The drawback of this approach is that there will be useless memory on the stack in the form of global variables past the lifetime of the tests in the context. There is also a drawback that if any of the tests inside the context fails when they must pass, then the context cleanup procedure will not run, leaving the global memory allocated past its lifetime. This is another good reason not to compile the tests together with the release binary, but instead build a test binary and a release binary separately. There are plans to address these shortcomings at a later time.

## Future work
A context which can be used to set up data fixtures to be shared between tests could be useful. This type of context will have automatic initialization and cleanup, and will not leave data on the stack past the execution of tests inside the context.
