#ifndef UTEST_H_INCLUDED__
#define UTEST_H_INCLUDED__

#include <cstdint>
#include <iostream>

/// @brief Begins the definition of a user-defined unit test.
/// @param unit_class The class name of the unit test. Be descriptive as this will be the basis for the description of the test.
/// @note Use camel case for unit_class class name for automatic description formatting.
/// @sa UTEST_END
#define UTEST_BEGIN(unit_class) \
	class unit_class : public utest::UTestBase { \
	public: \
		unit_class( void ) : UTestBase()

/// @brief Ends a user-defined unit test.
/// @param unit_class The same class name used when beginning to define the test using UTEST_BEGIN.
/// @param must_pass If true, halts further tests from running within this context when this test fails.
/// @sa UTEST_BEGIN
#define UTEST_END(unit_class, must_pass) \
	}; \
	static bool run_##unit_class( void ) { return unit_class().Succeeded(); } \
	const static bool unit_class##_fn_added = utest::AddTest(run_##unit_class, #unit_class, __FILE__, must_pass);

/// @brief Exits a unit test if the test is not true.
/// @param l The left operand.
/// @param op The comparison operator (or any other operator that will yeild a boolean expression).
/// @param r The right operand.
#define UTEST_ASSERT(l, op, r) \
	IncrementAssertCount(); \
	if (!((l) op (r))) { \
		Fail(); \
		std::cout << "\n    #" << AssertCount() << " @" << __LINE__ << ": <<" << (l) << " " << #op << " " << (r) << ">> is false"; \
		return; \
	}

namespace utest
{
	/// @brief The base class for all unit tests.
	/// @note Do not use this class directly. Use UTEST_BEGIN and UTEST_END instead.
	/// @sa UTEST_BEGIN
	/// @sa UTEST_END
	class UTestBase
	{
	private:
		uint64_t m_assert_count;
		bool     m_success;

	protected:
		/// @brief Increments the number of asserts performed.
		void IncrementAssertCount( void );

		/// @brief Returns the current assert count.
		/// @return The current assert count.
		uint64_t AssertCount( void ) const;

		/// @brief Fails the test. 
		void Fail( void );

	public:
		/// @brief Default constructor. 
		UTestBase( void );

		/// @brief  Checks for success.
		/// @return True if the test succeeded.
		/// @sa Failed
		bool Succeeded( void ) const;

		/// @brief Checks for failure.
		/// @return True if the test failed.
		/// @sa Succeeded
		bool Failed( void ) const;
	};

	/// @brief Adds a test to the suite.
	/// @param fn The function pointer to the wrapper running the test.
	/// @param name The name of the test.
	/// @param context An identifier to group the test together with other tests. UTEST_END automatically uses the current filename as context.
	/// @param test_must_pass Controls whether the test must pass for the execution of the test suite to proceed.
	/// @return Always returns TRUE.
	/// @note Do not use this function directly. See instead UTEST_BEGIN and UTEST_END.
	/// @sa UTEST_BEGIN
	/// @sa UTEST_END
	bool AddTest(bool (*fn)(), const char *name, const char *context, bool test_must_pass);

	/// @brief Runs all tests defined using UTEST_BEGIN and UTEST_END.
	/// @return The return code of the execution. 0 for all-good, 1 for some failure.
	int Run( void );

	/// @brief Runs the tests inside the specified contexts that have been defined using UTEST_BEGIN and UTEST_END.
	/// @param contexts The names of the contexts run. UTEST_END automatically uses tests' filenames as context names so this is essentially the filenames of the source files containing the tests that should be executed.
	/// @param count The number of contexts passed.
	/// @return The return code of the execution. 0 for all-good, 1 for some failure.
	int Run(const char **contexts, uint32_t count);
}

#endif
