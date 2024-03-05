/// @file
/// @author github.com/SirJonthe
/// @date 2023
/// @copyright Public domain.
/// @license CC0 1.0

#include <iostream>
#include <string>
#include "utest.h"

void cc0::utest::UTestBase::IncrementAssertCount( void )
{
	++m_assert_count;
}

uint64_t cc0::utest::UTestBase::AssertCount( void ) const
{
	return m_assert_count;
}

void cc0::utest::UTestBase::Fail( void )
{
	m_success = false;
}

cc0::utest::UTestBase::UTestBase( void ) : m_assert_count(0), m_success(true)
{}

bool cc0::utest::UTestBase::Succeeded( void ) const
{
	return m_success;
}

bool cc0::utest::UTestBase::Failed( void ) const
{
	return !m_success;
}

template < typename type_t >
struct Item
{
	type_t       *data;
	Item<type_t> *next;

	Item(type_t *dat) : data(dat), next(nullptr) {}
	~Item( void ) {
		delete data;
		delete next;
	}
};

template < typename type_t >
struct List
{
	Item<type_t> *first;
	Item<type_t> *last;

	List( void ) : first(nullptr), last(nullptr) {}
	~List( void ) { delete first; }

	void Add(type_t *item) {
		Item<type_t> *i = new Item<type_t>(item);
		if (first == nullptr) {
			first = i;
			last = first;
		} else {
			last->next = i;
			last = last->next;
		}
	}
};

struct TestItem
{
	bool        (*test)();
	std::string name;
	bool        must_pass;

	TestItem(bool (*fn)(), const char *test_name, bool test_must_pass = false) : test(fn), name(test_name), must_pass(test_must_pass) {}
};

struct ContextItem
{
	std::string    name;
	bool           (*init)();
	bool           (*cleanup)();
	List<TestItem> tests;
	uint32_t       align_chars;

	explicit ContextItem(const char *context_name) : name(context_name), init(nullptr), cleanup(nullptr), align_chars(0) {}
};

struct ContextList
{
	List<ContextItem>  list;
	ContextItem       *last_used;
};

static ContextList &Contexts( void ) {
	static ContextList tests;
	return tests;
}

static ContextItem *FindContext(const char *name)
{
	ContextList &contexts = Contexts();
	if (contexts.last_used == nullptr || contexts.last_used->name != name) {
		Item<ContextItem> *c = contexts.list.first;
		while (c != nullptr) {
			if (c->data->name == name) {
				break;
			}
			c = c->next;
		}
		contexts.last_used = c != nullptr ? c->data : nullptr;
	}
	return contexts.last_used;
}

static ContextItem *FindOrAddContext(const char *name)
{
	ContextList &contexts = Contexts();
	FindContext(name);
	if (contexts.last_used == nullptr) {
		contexts.list.Add(new ContextItem(name));
		contexts.last_used = contexts.list.last->data;
	}
	return contexts.last_used;
}

static void PrintTestName(const std::string &class_name, uint32_t align_chars)
{
	std::cout << "  ";
	for (size_t i = 0, j = 0; i < class_name.size(); i = j) {
		j = class_name.find('_', i);
		std::cout << class_name.substr(i, (j - i)) << " ";
		if (j < std::string::npos) {
			++j;
		}
	}
	std::cout << "\x8";
	for (uint32_t i = 0; i < align_chars - class_name.size(); ++i) {
		std::cout << ".";
	}
	std::cout << std::flush;
}

static bool RunTests(List<TestItem> &tests, uint32_t align_chars)
{
	bool status = true;
	for (auto i = tests.first; i != nullptr; i = i->next) {
		PrintTestName(i->data->name, align_chars);
		if (!i->data->test()) {
			status = false;
			std::cout << "\n    fail" << std::endl;
			if (i->data->must_pass) {
				std::cout << "  [abort]" << std::endl;
				break;
			}
		} else {
			std::cout << "ok" << std::endl;
		}
	}
	return status;
}

static bool RunContext(ContextItem *c)
{
	bool status = true;
	std::cout << c->name << std::endl;
	if ((c->init != nullptr && !c->init()) || !RunTests(c->tests, c->align_chars)) {
		status = false;
	}
	if (c->cleanup != nullptr && !c->cleanup()) {
		status = false;
	}
	std::cout << "  " << (status ? "[ok]" : "[fail]") << std::endl;
	return status;
}

bool cc0::utest::AddTest(bool (*fn)(), const char *name, const char *context, bool test_must_pass)
{
	ContextItem *c = FindOrAddContext(context);
	c->tests.Add(new TestItem(fn, name, test_must_pass));
	c->align_chars = c->align_chars > uint32_t(c->tests.last->data->name.size()) + 3 ? c->align_chars : uint32_t(c->tests.last->data->name.size()) + 3;
	return true;
}

int cc0::utest::Run( void )
{
	bool status = true;
	for (auto c = Contexts().list.first; c != nullptr; c = c->next) {
		if (!RunContext(c->data)) {
			status = false;
		}
	}
	return status ? 0 : 1;
}

int cc0::utest::Run(const char **contexts, uint32_t count)
{
	bool status = true;
	for (uint32_t c = 0; c < count; ++c) {
		FindContext(contexts[c]);
		if (Contexts().last_used != nullptr) {
			if (!RunContext(Contexts().last_used)) {
				status = false;
			}
		} else {
			std::cout << contexts[c] << "...not found" << std::endl;
			status = false;
		}
	}
	return status ? 0 : 1;
}
