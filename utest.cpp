#include <iostream>
#include <string>
#include "utest.h"

void PrintName(const std::string &class_name)
{
	std::cout << "  ";
	for (size_t i = 0, j = 0; i < class_name.size(); i = j) {
		j = class_name.find('_', i);
		std::cout << class_name.substr(i, (j - i)) << " ";
		if (j < std::string::npos) {
			++j;
		}
	}
	std::cout << "\x8...\n";
}

utest::UTestBase::UTestBase(const char *class_name, uint64_t str_len) : m_assert_count(0), m_success(true)
{
	PrintName(std::string(class_name, str_len));
}
utest::UTestBase::~UTestBase( void )
{
	std::cout << "    " << (m_success ? "succeeded" : "failed") << std::endl;
}

bool utest::UTestBase::Succeeded( void ) const
{
	return m_success;
}

bool utest::UTestBase::Failed( void ) const
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
	bool (*test)();
	bool must_pass;

	TestItem(bool (*fn)(), bool test_must_pass = false) : test(fn), must_pass(test_must_pass) {}
};

struct ContextItem
{
	std::string    context_name;
	bool           (*init)();
	bool           (*cleanup)();
	List<TestItem> tests;

	explicit ContextItem(const char *context) : context_name(context), init(nullptr), cleanup(nullptr) {}
};

struct ContextList
{
	List<ContextItem>  list;
	ContextItem       *last_used;
};

ContextList &Contexts( void ) {
	static ContextList tests;
	return tests;
}

ContextItem *FindContext(const char *name)
{
	ContextList &contexts = Contexts();
	if (contexts.last_used == nullptr || contexts.last_used->context_name != name) {
		Item<ContextItem> *c = contexts.list.first;
		while (c != nullptr) {
			if (c->data->context_name == name) {
				break;
			}
			c = c->next;
		}
		contexts.last_used = c != nullptr ? c->data : nullptr;
	}
	return contexts.last_used;
}

ContextItem *FindOrAddContext(const char *name)
{
	ContextList &contexts = Contexts();
	FindContext(name);
	if (contexts.last_used == nullptr) {
		contexts.list.Add(new ContextItem(name));
		contexts.last_used = contexts.list.last->data;
	}
	return contexts.last_used;
}

bool RunTests(List<TestItem> &tests)
{
	bool status = true;
	for (auto i = tests.first; i != nullptr; i = i->next) {
		if (!i->data->test()) {
			status = false;
			if (i->data->must_pass) { break; }
		}
	}
	return status;
}

bool RunContext(ContextItem *c)
{
	bool status = true;
	std::cout << c->context_name << "..." << std::endl;
	if ((c->init != nullptr && !c->init()) || !RunTests(c->tests)) {
		status = false;
	}
	if (c->cleanup != nullptr && !c->cleanup()) {
		status = false;
	}
	std::cout << "  " << (status ? "succeeded" : "failed") << std::endl;
	return status;
}

bool utest::AddTest(bool (*fn)(), const char *context, bool test_must_pass)
{
	ContextItem *c = FindOrAddContext(context);
	c->tests.Add(new TestItem(fn, test_must_pass));
	return true;
}

int utest::Run( void )
{
	bool status = true;
	for (auto c = Contexts().list.first; c != nullptr; c = c->next) {
		if (!RunContext(c->data)) {
			status = false;
		}
	}
	return status ? 0 : 1;
}

int utest::Run(const char **contexts, uint32_t count)
{
	bool status = true;
	for (uint32_t c = 0; c < count; ++c) {
		FindContext(contexts[c]);
		if (Contexts().last_used != nullptr && !RunContext(Contexts().last_used)) {
			status = false;
		}
	}
	return status ? 0 : 1;
}
