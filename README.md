Pointer-To-Deep-data-Member
===========================
**ptdm** extends C++ built-in pointers-to-data-members and adds support of pointing to nested members (i.e. data members of a data member of a class, at any depth).

Dependencies
------------
- Requires C++20

License
-------
- **ptdm** is provided under the MIT license terms.

Quick start
-----------
```c++
#include "ptdm/ptdm.hpp"

template <typename T, ptdm::pointer P>
auto fn(T obj, P ptr)
{
    using ptdm::operator->*;
    return obj->*ptr;
}

void main {

    struct Object { int value; };
    struct Wrap { Object member; };

    Wrap wrapped {1};

    // pointer-to-deep-member from a lambda
    int result1 = fn(wrapped, [](const Wrap& from) -> const int& { return from.member.value; });

    // pointer-to-deep-member by chaining built-in pointer-to-members
    int result2 = fn(wrapped, ptdm::chain(&Wrap::member, &Object::value));

    // pointer-to-deep-member by chaining a built-in pointer-to-member and a lambda
    int result3 = fn(wrapped, ptdm::chain(&Wrap::member, [](const Object& from) -> const int& { return from.value; }));

    // operator| can be used instead of chain()
    int result4 = fn(wrapped, &Wrap::member | ptdm::wrap(&Object::value)); // wrap() stands to prevent operands from being both built-in types, which is disallowed for binary overloaded operators. It also enables ADL
    using ptdm::operator|;
    int result5 = fn(wrapped, &Wrap::member | [](const Object& from) -> const int& { return from.value; });

    return 0;
}
```

Features
--------

### The `pointer` concept
**ptdm** provides a flexible `pointer` concept that can match various forms of pointers-to-deep-data-member :
* `ptdm::pointer` matches any built-in pointer-to-data-member and any callable with `const&` argument and return type
* `ptdm::pointer<T>` restricts the above to pointers *from* type T, meaning builtins of type `X T::*` (X being any type), and callables with `const T&` argument
* `ptdm::pointer<T, U>` restricts the above to pointers *from* type T and *to* type U, meaning builtins of type `U T::*`, and callables with `const T&` argument and `const U&` return type
* `ptdm::pointer<ptdm::unconstrained, U>` restricts the above to pointers *to* type U, meaning builtins of type `U X::*` (X being any type), and callables with `const U&` return type

Note that `ptdm::pointer<ptdm::unconstrained, ptdm::unconstrained>` is equivalent to `ptdm::pointer`, and `ptdm::pointer<T, ptdm::unconstrained>` is equivalent to `ptdm::pointer<T>`.

### Retrieving *from* and *to* types of any `pointer`
**ptdm** provides two type aliases `ptdm::from<T>` and `ptdm::to<T>` that can be used to retrieve respectively the type the pointer can be applied onto, and the type of the member the pointer points towards.

* When applied to built-in pointer-to-data-members :
  * `ptdm::from<R C::*>` is `C`
  * `ptdm::to<R C::*>` is `R`

* When applied to callable `T` with `const C&` argument and `const R&` return type :
  * `ptdm::from<T>` is `C`
  * `ptdm::to<T>` is `R`

In any other situation, `ptdm::from<T>` and `ptdm::to<T>` have undefined behavior. A compile-time error may be thrown.

### Chaining pointers
**ptdm** enables the chaining of pointer-to-members in order to reach arbitrarily deep data members. This can be done in two equivalent ways :
* The `ptdm::chain()` function can take any number of `pointer` as arguments, and returns a `pointer` object that chains all arguments together
* The `ptdm::operator|` that achieves the exact same result, provided at least one of the operands is not a built-in pointer-to-member (C++ disallows binary overloaded operators to take 2 built-in types as arguments)

In both cases, arguments must be ordered such as the *to* type of `pointer` N is the same as the *from* type of `pointer` N+1.
The returned `pointer` object takes its *from* type from the 1st `pointer` passed as argument, and its *to* type from the last `pointer` passed as argument.

*Notes on `operator|` :*
* In most cases, you will need to manually bring the operator into the scope by `using ptdm::operator|;`. It is not required though if one of the arguments' type come from the `ptdm` namespace (as it unlocks symbol resolution via ADL). This is the case for pointers that have been chained or wrapped (see below)
* You can wrap any built-in pointer-to-member into the `pointer` type `ptdm::wrap`. Wrapping can be used to unlock the use of `operator|` even with only built-in arguments. It also enables ADL and may facilitate the use of the operator without having to rely on the `using ptdm::operator|;` directive

### Invoking pointers with `operator->*`
**ptdm** provides overloaded `ptdm::operator->*` that can handle any type satisfying `pointer`.
In most cases, you will need to manually bring the operator into the scope by `using ptdm::operator->*;`. It is not required though if the `pointer` type come from the `ptdm` namespace (as it unlocks symbol resolution via ADL). This is the case for pointers that have been chained or wrapped.

`ptdm::operator->*` return type depends on the object it's applied onto :
* if `object` is `const`-qualified, then `object->*p` returns a `const&` to the member
* otherwise, `object->*p` returns a `&` to the member

How to contribute
-----------------
- To report bugs : please open an issue in the current github project.
- To propose new features : please open an issue to allow room for discussion first. Eventually submit a pull request then.
