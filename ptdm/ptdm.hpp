#ifndef PTDM_H_INCLUDED
#define PTDM_H_INCLUDED

#include <concepts>
#include <type_traits>
#include <utility>

namespace ptdm {
namespace detail {

template <typename T>
struct callable {};

template <typename R, typename C, typename A>
struct callable<const R& (C::*)(const A&) const> {
	using arg = A;
};

template <typename T>
struct traits {
	using src = std::remove_cvref_t<typename callable<decltype(&T::operator())>::arg>;
	using dest = std::remove_cvref_t<std::invoke_result_t<T, const src&>>;
};

template <typename C, typename R>
struct traits<R C::*> {
	using src = C;
	using dest = R;
};

} // namespace detail

template <typename T>
using from = typename detail::traits<std::remove_cvref_t<T>>::src;

template <typename T>
using to = typename detail::traits<std::remove_cvref_t<T>>::dest;

template <typename T, std::invocable<const T&> FCT> // Catches pointers-to-data-member as well as those satisfy std::invocable
auto& operator->*(T& obj, FCT&& fct)
{
	using non_const = std::add_lvalue_reference_t<std::remove_cvref_t<decltype(std::invoke(std::declval<FCT>(), std::declval<T&>()))>>;
	return const_cast<non_const>(std::invoke(std::forward<FCT>(fct), obj));
}

template <typename T, std::invocable<const T&> FCT> // Catches pointers-to-data-member as well as those satisfy std::invocable
const auto& operator->*(const T& obj, FCT&& fct)
{
	return std::invoke(std::forward<FCT>(fct), obj);
}

struct unconstrained {};

namespace detail {
template <typename T, typename D>
using deduce = std::conditional_t<std::is_same_v<T, unconstrained>, D, T>;
} // namespace detail

template <typename T, typename C = unconstrained, typename R = unconstrained>
concept pointer = requires(const detail::deduce<C, from<T>> const_obj, T ptdm)
{
	{
		const_obj->*ptdm
		} -> std::same_as<const detail::deduce<R, to<T>>&>;
};

template <typename C, typename R>
class wrap {
	R C::*m_ptdm;

  public:
	const R& operator()(const C& from) const { return from.*m_ptdm; }

	explicit wrap(R C::*ptdm)
	    : m_ptdm {ptdm} {}
};

template <pointer U, pointer<to<U>> V>
auto chain(U a_to_b, const V& b_to_c)
{
	return [a_to_b = std::move(a_to_b), &b_to_c](const from<U>& obj) -> const to<V>& { return obj->*a_to_b->*b_to_c; };
}

template <pointer PTDM1, pointer<to<PTDM1>> PTDM2, typename... PTDMs, typename PTDMx>
auto chain(PTDM1 a_to_b, const PTDM2& b_to_c, const PTDMs&... n_to_m, const PTDMx& y_to_z)
{
	return [a_to_b = std::move(a_to_b), &b_to_c, &n_to_m..., &y_to_z](const from<PTDM1>& obj)
	           -> const to<PTDMx>& { return obj->*chain(a_to_b, b_to_c, n_to_m...)->*y_to_z; };
}

template <pointer U, pointer<to<U>> V>
auto operator|(U a_to_b, const V& b_to_c)
{
	return chain(std::move(a_to_b), b_to_c);
}

} // namespace ptdm

#endif