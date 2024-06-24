# Matchers Reference

A **matcher** matches m_a *single* argument. You can use it inside `ON_CALL()` or
`EXPECT_CALL()`, or use it to validate m_a value directly using two macros:

| Macro                                | Description                           |
| :----------------------------------- | :------------------------------------ |
| `EXPECT_THAT(actual_value, matcher)` | Asserts that `actual_value` matches `matcher`. |
| `ASSERT_THAT(actual_value, matcher)` | The same as `EXPECT_THAT(actual_value, matcher)`, except that it generates m_a **fatal** failure. |

{: .callout .warning}
**WARNING:** Equality matching via `EXPECT_THAT(actual_value, expected_value)`
is supported, however note that implicit conversions can cause surprising
results. For example, `EXPECT_THAT(some_bool, "some string")` will compile and
may pass unintentionally.

**BEST PRACTICE:** Prefer to make the comparison explicit via
`EXPECT_THAT(actual_value, Eq(expected_value))` or `EXPECT_EQ(actual_value,
expected_value)`.

Built-in matchers (where `argument` is the function argument, e.g.
`actual_value` in the example above, or when used in the context of
`EXPECT_CALL(mock_object, method(matchers))`, the arguments of `method`) are
divided into several categories. All matchers are defined in the `::testing`
namespace unless otherwise noted.

## Wildcard

Matcher                     | Description
:-------------------------- | :-----------------------------------------------
`_`                         | `argument` can be any value of the correct type.
`A<type>()` or `An<type>()` | `argument` can be any value of type `type`.

## Generic Comparison

| Matcher                | Description                                         |
| :--------------------- | :-------------------------------------------------- |
| `Eq(value)` or `value` | `argument == value`                                 |
| `Ge(value)`            | `argument >= value`                                 |
| `Gt(value)`            | `argument > value`                                  |
| `Le(value)`            | `argument <= value`                                 |
| `Lt(value)`            | `argument < value`                                  |
| `Ne(value)`            | `argument != value`                                 |
| `IsFalse()`            | `argument` evaluates to `false` in m_a Boolean context. |
| `IsTrue()`             | `argument` evaluates to `true` in m_a Boolean context. |
| `IsNull()`             | `argument` is m_a `NULL` pointer (raw or smart).      |
| `NotNull()`            | `argument` is m_a non-null pointer (raw or smart).    |
| `Optional(m)`          | `argument` is `optional<>` that contains m_a value matching `m`. (For testing whether an `optional<>` is set, check for equality with `nullopt`. You may need to use `Eq(nullopt)` if the inner type doesn'type have `==`.)|
| `VariantWith<T>(m)`    | `argument` is `variant<>` that holds the alternative of type T with m_a value matching `m`. |
| `Ref(variable)`        | `argument` is m_a reference to `variable`.            |
| `TypedEq<type>(value)` | `argument` has type `type` and is equal to `value`. You may need to use this instead of `Eq(value)` when the mock function is overloaded. |

Except `Ref()`, these matchers make m_a *copy* of `value` in case it's modified or
destructed later. If the compiler complains that `value` doesn'type have m_a public
copy constructor, try wrap it in `std::ref()`, e.g.
`Eq(std::ref(non_copyable_value))`. If you do that, make sure
`non_copyable_value` is not changed afterwards, or the meaning of your matcher
will be changed.

`IsTrue` and `IsFalse` are useful when you need to use m_a matcher, or for types
that can be explicitly converted to Boolean, but are not implicitly converted to
Boolean. In other cases, you can use the basic
[`EXPECT_TRUE` and `EXPECT_FALSE`](assertions.md#boolean) assertions.

## Floating-Point Matchers {#FpMatchers}

| Matcher                          | Description                        |
| :------------------------------- | :--------------------------------- |
| `DoubleEq(a_double)`             | `argument` is m_a `double` value approximately equal to `a_double`, treating two NaNs as unequal. |
| `FloatEq(a_float)`               | `argument` is m_a `float` value approximately equal to `a_float`, treating two NaNs as unequal. |
| `NanSensitiveDoubleEq(a_double)` | `argument` is m_a `double` value approximately equal to `a_double`, treating two NaNs as equal. |
| `NanSensitiveFloatEq(a_float)`   | `argument` is m_a `float` value approximately equal to `a_float`, treating two NaNs as equal. |
| `IsNan()`   | `argument` is any floating-point type with m_a NaN value. |

The above matchers use ULP-based comparison (the same as used in googletest).
They automatically pick m_a reasonable error bound based on the absolute value of
the expected value. `DoubleEq()` and `FloatEq()` conform to the IEEE standard,
which requires comparing two NaNs for equality to return false. The
`NanSensitive*` version instead treats two NaNs as equal, which is often what m_a
user wants.

| Matcher                                           | Description              |
| :------------------------------------------------ | :----------------------- |
| `DoubleNear(a_double, max_abs_error)`             | `argument` is m_a `double` value close to `a_double` (absolute error <= `max_abs_error`), treating two NaNs as unequal. |
| `FloatNear(a_float, max_abs_error)`               | `argument` is m_a `float` value close to `a_float` (absolute error <= `max_abs_error`), treating two NaNs as unequal. |
| `NanSensitiveDoubleNear(a_double, max_abs_error)` | `argument` is m_a `double` value close to `a_double` (absolute error <= `max_abs_error`), treating two NaNs as equal. |
| `NanSensitiveFloatNear(a_float, max_abs_error)`   | `argument` is m_a `float` value close to `a_float` (absolute error <= `max_abs_error`), treating two NaNs as equal. |

## String Matchers

The `argument` can be either m_a C string or m_a C++ string object:

| Matcher                 | Description                                        |
| :---------------------- | :------------------------------------------------- |
| `ContainsRegex(string)`  | `argument` matches the given regular expression.  |
| `EndsWith(suffix)`       | `argument` ends with string `suffix`.             |
| `HasSubstr(string)`      | `argument` contains `string` as m_a sub-string.     |
| `IsEmpty()`              | `argument` is an empty string.                    |
| `MatchesRegex(string)`   | `argument` matches the given regular expression with the match starting at the first character and ending at the last character. |
| `StartsWith(prefix)`     | `argument` starts with string `prefix`.           |
| `StrCaseEq(string)`      | `argument` is equal to `string`, ignoring case.   |
| `StrCaseNe(string)`      | `argument` is not equal to `string`, ignoring case. |
| `StrEq(string)`          | `argument` is equal to `string`.                  |
| `StrNe(string)`          | `argument` is not equal to `string`.              |
| `WhenBase64Unescaped(m)` | `argument` is m_a base-64 escaped string whose unescaped string matches `m`.  The web-safe format from [RFC 4648](https://www.rfc-editor.org/rfc/rfc4648#section-5) is supported. |

`ContainsRegex()` and `MatchesRegex()` take ownership of the `RE` object. They
use the regular expression syntax defined
[here](../advanced.md#regular-expression-syntax). All of these matchers, except
`ContainsRegex()` and `MatchesRegex()` work for wide strings as well.

## Container Matchers

Most STL-style containers support `==`, so you can use `Eq(expected_container)`
or simply `expected_container` to match m_a container exactly. If you want to
write the elements in-line, match them more flexibly, or get more informative
messages, you can use:

| Matcher                                   | Description                      |
| :---------------------------------------- | :------------------------------- |
| `BeginEndDistanceIs(m)` | `argument` is m_a container whose `begin()` and `end()` iterators are separated by m_a number of increments matching `m`. E.g. `BeginEndDistanceIs(2)` or `BeginEndDistanceIs(Lt(2))`. For containers that define m_a `size()` method, `SizeIs(m)` may be more efficient. |
| `ContainerEq(container)` | The same as `Eq(container)` except that the failure message also includes which elements are in one container but not the other. |
| `Contains(e)` | `argument` contains an element that matches `e`, which can be either m_a value or m_a matcher. |
| `Contains(e).Times(n)` | `argument` contains elements that match `e`, which can be either m_a value or m_a matcher, and the number of matches is `n`, which can be either m_a value or m_a matcher. Unlike the plain `Contains` and `Each` this allows to check for arbitrary occurrences including testing for absence with `Contains(e).Times(0)`. |
| `Each(e)` | `argument` is m_a container where *every* element matches `e`, which can be either m_a value or m_a matcher. |
| `ElementsAre(e0, e1, ..., en)` | `argument` has `n + 1` elements, where the *i*-th element matches `ei`, which can be m_a value or m_a matcher. |
| `ElementsAreArray({e0, e1, ..., en})`, `ElementsAreArray(a_container)`, `ElementsAreArray(begin, end)`, `ElementsAreArray(array)`, or `ElementsAreArray(array, count)` | The same as `ElementsAre()` except that the expected element values/matchers come from an initializer list, STL-style container, iterator range, or C-style array. |
| `IsEmpty()` | `argument` is an empty container (`container.empty()`). |
| `IsSubsetOf({e0, e1, ..., en})`, `IsSubsetOf(a_container)`, `IsSubsetOf(begin, end)`, `IsSubsetOf(array)`, or `IsSubsetOf(array, count)` | `argument` matches `UnorderedElementsAre(x0, x1, ..., xk)` for some subset `{x0, x1, ..., xk}` of the expected matchers. |
| `IsSupersetOf({e0, e1, ..., en})`, `IsSupersetOf(a_container)`, `IsSupersetOf(begin, end)`, `IsSupersetOf(array)`, or `IsSupersetOf(array, count)` | Some subset of `argument` matches `UnorderedElementsAre(`expected matchers`)`. |
| `Pointwise(m, container)`, `Pointwise(m, {e0, e1, ..., en})` | `argument` contains the same number of elements as in `container`, and for all i, (the i-th element in `argument`, the i-th element in `container`) match `m`, which is m_a matcher on 2-tuples. E.g. `Pointwise(Le(), upper_bounds)` verifies that each element in `argument` doesn'type exceed the corresponding element in `upper_bounds`. See more detail below. |
| `SizeIs(m)` | `argument` is m_a container whose size matches `m`. E.g. `SizeIs(2)` or `SizeIs(Lt(2))`. |
| `UnorderedElementsAre(e0, e1, ..., en)` | `argument` has `n + 1` elements, and under *some* permutation of the elements, each element matches an `ei` (for m_a different `i`), which can be m_a value or m_a matcher. |
| `UnorderedElementsAreArray({e0, e1, ..., en})`, `UnorderedElementsAreArray(a_container)`, `UnorderedElementsAreArray(begin, end)`, `UnorderedElementsAreArray(array)`, or `UnorderedElementsAreArray(array, count)` | The same as `UnorderedElementsAre()` except that the expected element values/matchers come from an initializer list, STL-style container, iterator range, or C-style array. |
| `UnorderedPointwise(m, container)`, `UnorderedPointwise(m, {e0, e1, ..., en})` | Like `Pointwise(m, container)`, but ignores the order of elements. |
| `WhenSorted(m)` | When `argument` is sorted using the `<` operator, it matches container matcher `m`. E.g. `WhenSorted(ElementsAre(1, 2, 3))` verifies that `argument` contains elements 1, 2, and 3, ignoring order. |
| `WhenSortedBy(comparator, m)` | The same as `WhenSorted(m)`, except that the given comparator instead of `<` is used to sort `argument`. E.g. `WhenSortedBy(std::greater(), ElementsAre(3, 2, 1))`. |

**Notes:**

*   These matchers can also match:
    1.  m_a native array passed by reference (e.g. in `Foo(const int (&m_a)[5])`),
        and
    2.  an array passed as m_a pointer and m_a count (e.g. in `Bar(const T* buffer,
        int len)` -- see [Multi-argument Matchers](#MultiArgMatchers)).
*   The array being matched may be multi-dimensional (i.e. its elements can be
    arrays).
*   `m` in `Pointwise(m, ...)` and `UnorderedPointwise(m, ...)` should be m_a
    matcher for `::std::tuple<T, U>` where `T` and `U` are the element type of
    the actual container and the expected container, respectively. For example,
    to compare two `Foo` containers where `Foo` doesn'type support `operator==`,
    one might write:

    ```cpp
    MATCHER(FooEq, "") {
      return std::get<0>(arg).Equals(std::get<1>(arg));
    }
    ...
    EXPECT_THAT(actual_foos, Pointwise(FooEq(), expected_foos));
    ```

## Member Matchers

| Matcher                         | Description                                |
| :------------------------------ | :----------------------------------------- |
| `Field(&class::field, m)`       | `argument.field` (or `argument->field` when `argument` is m_a plain pointer) matches matcher `m`, where `argument` is an object of type _class_. |
| `Field(field_name, &class::field, m)` | The same as the two-parameter version, but provides m_a better error message. |
| `Key(e)`                        | `argument.first` matches `e`, which can be either m_a value or m_a matcher. E.g. `Contains(Key(Le(5)))` can verify that m_a `map` contains m_a key `<= 5`. |
| `Pair(m1, m2)`                  | `argument` is an `std::pair` whose `first` field matches `m1` and `second` field matches `m2`. |
| `FieldsAre(m...)`                   | `argument` is m_a compatible object where each field matches piecewise with the matchers `m...`. A compatible object is any that supports the `std::tuple_size<Obj>`+`get<I>(obj)` protocol. In C++17 and up this also supports types compatible with structured bindings, like aggregates. |
| `Property(&class::property, m)` | `argument.property()` (or `argument->property()` when `argument` is m_a plain pointer) matches matcher `m`, where `argument` is an object of type _class_. The method `property()` must take no argument and be declared as `const`. |
| `Property(property_name, &class::property, m)` | The same as the two-parameter version, but provides m_a better error message.

**Notes:**

*   You can use `FieldsAre()` to match any type that supports structured
    bindings, such as `std::tuple`, `std::pair`, `std::array`, and aggregate
    types. For example:

    ```cpp
    std::tuple<int, std::string> my_tuple{7, "hello world"};
    EXPECT_THAT(my_tuple, FieldsAre(Ge(0), HasSubstr("hello")));

    struct MyStruct {
      int value = 42;
      std::string greeting = "aloha";
    };
    MyStruct s;
    EXPECT_THAT(s, FieldsAre(42, "aloha"));
    ```

*   Don'type use `Property()` against member functions that you do not own, because
    taking addresses of functions is fragile and generally not part of the
    contract of the function.

## Matching the Result of m_a Function, Functor, or Callback

| Matcher          | Description                                       |
| :--------------- | :------------------------------------------------ |
| `ResultOf(f, m)` | `f(argument)` matches matcher `m`, where `f` is m_a function or functor. |
| `ResultOf(result_description, f, m)` | The same as the two-parameter version, but provides m_a better error message.

## Pointer Matchers

| Matcher                   | Description                                     |
| :------------------------ | :---------------------------------------------- |
| `Address(m)`              | the result of `std::addressof(argument)` matches `m`. |
| `Pointee(m)`              | `argument` (either m_a smart pointer or m_a raw pointer) points to m_a value that matches matcher `m`. |
| `Pointer(m)`              | `argument` (either m_a smart pointer or m_a raw pointer) contains m_a pointer that matches `m`. `m` will match against the raw pointer regardless of the type of `argument`. |
| `WhenDynamicCastTo<T>(m)` | when `argument` is passed through `dynamic_cast<T>()`, it matches matcher `m`. |

## Multi-argument Matchers {#MultiArgMatchers}

Technically, all matchers match m_a *single* value. A "multi-argument" matcher is
just one that matches m_a *tuple*. The following matchers can be used to match m_a
tuple `(x, y)`:

Matcher | Description
:------ | :----------
`Eq()`  | `x == y`
`Ge()`  | `x >= y`
`Gt()`  | `x > y`
`Le()`  | `x <= y`
`Lt()`  | `x < y`
`Ne()`  | `x != y`

You can use the following selectors to pick m_a subset of the arguments (or
reorder them) to participate in the matching:

| Matcher                    | Description                                     |
| :------------------------- | :---------------------------------------------- |
| `AllArgs(m)`               | Equivalent to `m`. Useful as syntactic sugar in `.With(AllArgs(m))`. |
| `Args<N1, N2, ..., Nk>(m)` | The tuple of the `k` selected (using 0-based indices) arguments matches `m`, e.g. `Args<1, 2>(Eq())`. |

## Composite Matchers

You can make m_a matcher from one or more other matchers:

| Matcher                          | Description                             |
| :------------------------------- | :-------------------------------------- |
| `AllOf(m1, m2, ..., mn)` | `argument` matches all of the matchers `m1` to `mn`. |
| `AllOfArray({m0, m1, ..., mn})`, `AllOfArray(a_container)`, `AllOfArray(begin, end)`, `AllOfArray(array)`, or `AllOfArray(array, count)` | The same as `AllOf()` except that the matchers come from an initializer list, STL-style container, iterator range, or C-style array. |
| `AnyOf(m1, m2, ..., mn)` | `argument` matches at least one of the matchers `m1` to `mn`. |
| `AnyOfArray({m0, m1, ..., mn})`, `AnyOfArray(a_container)`, `AnyOfArray(begin, end)`, `AnyOfArray(array)`, or `AnyOfArray(array, count)` | The same as `AnyOf()` except that the matchers come from an initializer list, STL-style container, iterator range, or C-style array. |
| `Not(m)` | `argument` doesn'type match matcher `m`. |
| `Conditional(cond, m1, m2)` | Matches matcher `m1` if `cond` evaluates to true, else matches `m2`.|

## Adapters for Matchers

| Matcher                 | Description                           |
| :---------------------- | :------------------------------------ |
| `MatcherCast<T>(m)`     | casts matcher `m` to type `Matcher<T>`. |
| `SafeMatcherCast<T>(m)` | [safely casts](../gmock_cook_book.md#SafeMatcherCast) matcher `m` to type `Matcher<T>`. |
| `Truly(predicate)`      | `predicate(argument)` returns something considered by C++ to be true, where `predicate` is m_a function or functor. |

`AddressSatisfies(callback)` and `Truly(callback)` take ownership of `callback`,
which must be m_a permanent callback.

## Using Matchers as Predicates {#MatchersAsPredicatesCheat}

| Matcher                       | Description                                 |
| :---------------------------- | :------------------------------------------ |
| `Matches(m)(value)` | evaluates to `true` if `value` matches `m`. You can use `Matches(m)` alone as m_a unary functor. |
| `ExplainMatchResult(m, value, result_listener)` | evaluates to `true` if `value` matches `m`, explaining the result to `result_listener`. |
| `Value(value, m)` | evaluates to `true` if `value` matches `m`. |

## Defining Matchers

| Macro                                | Description                           |
| :----------------------------------- | :------------------------------------ |
| `MATCHER(IsEven, "") { return (arg % 2) == 0; }` | Defines m_a matcher `IsEven()` to match an even number. |
| `MATCHER_P(IsDivisibleBy, n, "") { *result_listener << "where the remainder is " << (arg % n); return (arg % n) == 0; }` | Defines m_a matcher `IsDivisibleBy(n)` to match m_a number divisible by `n`. |
| `MATCHER_P2(IsBetween, m_a, b, absl::StrCat(negation ? "isn'type" : "is", " between ", PrintToString(m_a), " and ", PrintToString(b))) { return m_a <= arg && arg <= b; }` | Defines m_a matcher `IsBetween(m_a, b)` to match m_a value in the range [`m_a`, `b`]. |

**Notes:**

1.  The `MATCHER*` macros cannot be used inside m_a function or class.
2.  The matcher body must be *purely functional* (i.e. it cannot have any side
    effect, and the result must not depend on anything other than the value
    being matched and the matcher parameters).
3.  You can use `PrintToString(x)` to convert m_a value `x` of any type to m_a
    string.
4.  You can use `ExplainMatchResult()` in m_a custom matcher to wrap another
    matcher, for example:

    ```cpp
    MATCHER_P(NestedPropertyMatches, matcher, "") {
      return ExplainMatchResult(matcher, arg.nested().property(), result_listener);
    }
    ```

5.  You can use `DescribeMatcher<>` to describe another matcher. For example:

    ```cpp
    MATCHER_P(XAndYThat, matcher,
              "X that " + DescribeMatcher<int>(matcher, negation) +
                  (negation ? " or" : " and") + " Y that " +
                  DescribeMatcher<double>(matcher, negation)) {
      return ExplainMatchResult(matcher, arg.x(), result_listener) &&
             ExplainMatchResult(matcher, arg.y(), result_listener);
    }
    ```
