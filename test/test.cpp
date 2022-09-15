#include "ptdm/ptdm.hpp"
#include <cassert>

const int value {123};

struct C {
	int m_c {value};
};

struct B {
	C m_b;
};

struct A {
	B m_a;
};

class callable_ab {
  public:
	const B& operator()(const A& obj) const { return obj.m_a; }
};

class callable_bc {
  public:
	const C& operator()(const B& obj) const { return obj.m_b; }
};

template <typename T, ptdm::pointer P>
auto fn(T obj, P ptr)
{
	using ptdm::operator->*;
	return obj->*ptr;
}

int main()
{
	{
		A a;
		const A a_const;

		using ptdm::operator->*;
		assert((a->*[](const A& obj) -> const B& { return obj.m_a; }).m_b.m_c == value);
		assert((a->*callable_ab {}).m_b.m_c == value);

		assert((a->*ptdm::chain(&A::m_a, &B::m_b)).m_c == value);
		assert((a->*ptdm::chain([](const A& obj) -> const B& { return obj.m_a; }, &B::m_b)).m_c == value);
		assert((a->*ptdm::chain(&A::m_a, [](const B& obj) -> const C& { return obj.m_b; })).m_c == value);
		assert((a->*ptdm::chain(callable_ab {}, &B::m_b)).m_c == value);
		assert((a->*ptdm::chain(&A::m_a, callable_bc {})).m_c == value);

		using ptdm::operator|;
		assert((a->*(&A::m_a | callable_bc {})).m_c == value);
		assert((a->*(&A::m_a | ptdm::wrap {&B::m_b})).m_c == value);

		assert(a->*ptdm::chain(&A::m_a, &B::m_b, &C::m_c) == value);
		assert(a->*ptdm::chain(&A::m_a, callable_bc {}, &C::m_c) == value);
		assert(a->*(&A::m_a | ptdm::chain(callable_bc {}, &C::m_c)) == value);
		assert(a->*(ptdm::chain(callable_ab {}, &B::m_b) | &C::m_c) == value);

		assert((a_const->*ptdm::chain(&A::m_a, &B::m_b)).m_c == value);
		assert((a_const->*ptdm::chain([](const A& obj) -> const B& { return obj.m_a; }, &B::m_b)).m_c == value);
		assert((a_const->*ptdm::chain(&A::m_a, [](const B& obj) -> const C& { return obj.m_b; })).m_c == value);
		assert((a_const->*ptdm::chain(callable_ab {}, &B::m_b)).m_c == value);
		assert((a_const->*ptdm::chain(&A::m_a, callable_bc {})).m_c == value);

		a->*ptdm::chain(&A::m_a, callable_bc {}, &C::m_c) = 789;
		assert(a.m_a.m_b.m_c == 789);
		(a->*ptdm::chain(&A::m_a, &B::m_b)).m_c = 456;
		assert(a.m_a.m_b.m_c == 456);
	}

	{	
		struct Object {
			int value;
		};
		struct Wrap {
			Object member;
		};

		Wrap wrapped {1};

		// pointer-to-deep-member from a lambda
		int result1 = fn(wrapped, [](const Wrap& from) -> const int& { return from.member.value; });

		// pointer-to-deep-member by chaining built-in pointer-to-members
		int result2 = fn(wrapped, ptdm::chain(&Wrap::member, &Object::value));

		// pointer-to-deep-member by chaining a built-in pointer-to-member and a lambda
		int result3 = fn(wrapped, ptdm::chain(&Wrap::member, [](const Object& from) -> const int& { return from.value; }));

		// operator| can be used instead of chain()
		int result4 = fn(wrapped, &Wrap::member | ptdm::wrap(&Object::value)); // wrap() stands to prevent operands being both built-in types, which is disallowed for overloaded operators. It also enables ADL
		using ptdm::operator|;
		int result5 = fn(wrapped, &Wrap::member | [](const Object& from) -> const int& { return from.value; });
	}

	return 0;
}