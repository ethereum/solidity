## 0. Formatting

**GOLDEN RULE**: Follow the style of the existing code when you make changes.

1. Use tabs for leading indentation:
   - tab stops are every 4 characters (only relevant for line length).
   - one indentation level -> exactly one byte (i.e. a tab character) in the source file.
2. Line widths:
   - Lines should be at most 99 characters wide to make diff views readable and reduce merge conflicts.
   - Lines of comments should be formatted according to ease of viewing, but simplicity is to be preferred over beauty.
3. Single-statement blocks should not have braces, unless required for clarity.
4. Never place condition bodies on same line as condition.
5. Space between keyword and opening parenthesis, but not following opening parenthesis or before final parenthesis.
6. No spaces for unary operators, `->` or `.`.
7. No space before `:` but one after it, except in the ternary operator: one on both sides.
8. Add spaces around all other operators.
9. Braces, when used, always have their own lines and are at same indentation level as "parent" scope.
10. If lines are broken, a list of elements enclosed with parentheses (of any kind) and separated by a separator (of any kind) are formatted such that there is exactly one element per line, followed by the separator, the opening parenthesis is on the first line, followed by a line break and the closing parenthesis is on a line of its own unindented). See example below.

Yes:
```cpp
if (a == b[i])
    printf("Hello\n");	// NOTE spaces used instead of tab here for clarity - first byte should be '\t'.
foo->bar(
    someLongVariableName,
    anotherLongVariableName,
    anotherLongVariableName,
    anotherLongVariableName,
    anotherLongVariableName
);
cout <<
    "some very long string that contains completely irrelevant " <<
    "text that talks about this and that and contains the words " <<
    "\"lorem\" and \"ipsum\"" <<
    endl;
```

No:
```cpp
if( a==b[ i ] ) { printf ("Hello\n"); }
foo->bar(someLongVariableName,
         anotherLongVariableName,
         anotherLongVariableName,
         anotherLongVariableName,
         anotherLongVariableName);
cout << "some very long string that contains completely irrelevant text that talks about this and that and contains the words \"lorem\" and \"ipsum\"" << endl;
```

## 1. Namespaces

1. No `using namespace` declarations in header files.
2. Use `using namespace std;` in cpp files, but avoid importing namespaces from boost and others.
3. All symbols should be declared in a namespace except for final applications.
4. Use anonymous namespaces for helpers whose scope is a cpp file only.
5. Preprocessor symbols should be prefixed with the namespace in all-caps and an underscore.

Only in the header:
```cpp
#include <cassert>
namespace myNamespace
{
std::tuple<float, float> meanAndSigma(std::vector<float> const& _v);
}
```

Only in the cpp file:
```cpp
#include <cassert>
using namespace std;
tuple<float, float> myNamespace::meanAndSigma(vector<float> const& _v)
{
  // ...
}
```

## 2. Preprocessor

1. File comment is always at top, and includes:
   - Copyright
   - License (e.g. see COPYING)
2. Never use `#ifdef`/`#define`/`#endif` file guards. Prefer `#pragma` once as first line below file comment.
3. Prefer static const variable to value macros.
4. Prefer inline constexpr functions to function macros.
5. Split complex macro on multiple lines with `\`.

## 3. Capitalization

**GOLDEN RULE**: Preprocessor: `ALL_CAPS`; C++: `camelCase`.

1. Use camelCase for splitting words in names, except where obviously extending STL/boost functionality in which case follow those naming conventions.
2. The following entities' first alpha is upper case:
   - Type names
   - Template parameters
   - Enum members
   - static const variables that form an external API.
3. All preprocessor symbols (macros, macro arguments) in full uppercase with underscore word separation.

All other entities' first alpha is lower case.

## 4. Variable prefixes

1. Leading underscore "_" to parameter names:
   - Exception: "o_parameterName" when it is used exclusively for output. See 6(f).
   - Exception: "io_parameterName" when it is used for both input and output. See 6(f).
2. Leading "g_" to global (non-const) variables.
3. Leading "s_" to static (non-const, non-global) variables.

## 5. Assertions

Use `solAssert` and `solUnimplementedAssert` generously to check assumptions that span across different parts of the code base, for example before dereferencing a pointer.

## 6. Declarations

1. {Typename} + {qualifiers} + {name}.
2. Only one per line.
3. Associate */& with type, not variable (at ends with parser, but more readable, and safe if in conjunction with (b)).
4. Favour declarations close to use; don't habitually declare at top of scope ala C.
5. Pass non-trivial parameters as const reference, unless the data is to be copied into the function, then either pass by const reference or by value and use std::move.
6. If a function returns multiple values, use std::tuple (std::pair acceptable) or better introduce a struct type. Do not use */& arguments.
7. Use parameters of pointer type only if ``nullptr`` is a valid argument, use references otherwise. Often, ``boost::optional`` is better suited than a raw pointer.
8. Never use a macro where adequate non-preprocessor C++ can be written.
9. Only use ``auto`` if the type is very long and rather irrelevant.
10. Do not pass bools: prefer enumerations instead.
11. Prefer enum class to straight enum.
12. Always initialize POD variables, even if their value is overwritten later.

Yes:
```cpp
enum class Accuracy
{
	Approximate,
	Exact
};
struct MeanSigma
{
	float mean;
	float standardDeviation;
};
double const d = 0;
int i;
int j;
char* s;
MeanAndSigma ms meanAndSigma(std::vector<float> const& _v, Accuracy _a);
Derived* x = dynamic_cast<Derived*>(base);
for (auto i = x->begin(); i != x->end(); ++i) {}
```

No:
```cpp
const double d = 0;
int i, j;
char *s;
float meanAndSigma(std::vector<float> _v, float* _sigma, bool _approximate);
Derived* x(dynamic_cast<Derived*>(base));
for (map<ComplexTypeOne, ComplexTypeTwo>::iterator i = l.begin(); i != l.end(); ++l) {}
```

## 7. Structs & classes

1. Structs to be used when all members public and no virtual functions:
   - In this case, members should be named naturally and not prefixed with `m_`.
2. Classes to be used in all other circumstances.

## 8. Members

1. One member per line only.
2. Private, non-static, non-const fields prefixed with `m_`.
3. Avoid public fields, except in structs.
4. Use override, final and const as much as possible.
5. No implementations with the class declaration, except:
   - template or force-inline method (though prefer implementation at bottom of header file).
   - one-line implementation (in which case include it in same line as declaration).
6. For a property `foo`
   - Member: `m_foo`
   - Getter: `foo()` [ also: for booleans, `isFoo()` ]
   - Setter: `setFoo()`

## 9. Naming

1. Avoid unpronouncable names.
2. Names should be shortened only if they are extremely common, but shortening should be generally avoided
3. Avoid prefixes of initials (e.g. do not use `IMyInterface`, `CMyImplementation`)
4. Find short, memorable & (at least semi-) descriptive names for commonly used classes or name-fragments:
   - A dictionary and thesaurus are your friends;
   - Spell correctly;
   - Think carefully about the class's purpose;
   - Imagine it as an isolated component to try to decontextualise it when considering its name;
   - Don't be trapped into naming it (purely) in terms of its implementation.

## 10. Type definitions

1. Prefer `using` to `typedef`. e.g. `using ints = std::vector<int>;` rather than typedef `std::vector<int> ints;`
2. Generally avoid shortening a standard form that already includes all important information:
   - e.g. stick to `shared_ptr<X>` rather than shortening to `ptr<X>`.
3. Where there are exceptions to this (due to excessive use and clear meaning), note the change prominently and use it consistently:
   - e.g. `using Guard = std::lock_guard<std::mutex>;` ///< Guard is used throughout the codebase since it is clear in meaning and used commonly.
4. In general expressions should be roughly as important/semantically meaningful as the space they occupy.
5. Avoid introducing aliases for types unless they are very complicated. Consider the number of items a brain can keep track of at the same time.

## 11. Commenting

1. Comments should be doxygen-compilable, using @notation rather than \notation.
2. Document the interface, not the implementation:
   - Documentation should be able to remain completely unchanged, even if the method is reimplemented;
   - Comment in terms of the method properties and intended alteration to class state (or what aspects of the state it reports);
   - Be careful to scrutinise documentation that extends only to intended purpose and usage;
   - Reject documentation that is simply an English transaction of the implementation.
3. Avoid in-code comments. Instead, try to extract blocks of functionality into functions. This often already eliminates the need for an in-code comment.

## 12. Include Headers

1. Includes should go in increasing order of generality (`libsolidity` -> `libevmasm` -> `libdevcore` -> `boost` -> `STL`).
2. The corresponding `.h` file should be the first include in the respective `.cpp` file.
3. Insert empty lines between blocks of include files.

Example:
```cpp
#include <libsolidity/codegen/ExpressionCompiler.h>

#include <libsolidity/ast/AST.h>
#include <libsolidity/codegen/CompilerContext.h>
#include <libsolidity/codegen/CompilerUtils.h>
#include <libsolidity/codegen/LValue.h>

#include <libevmasm/GasMeter.h>

#include <libdevcore/Common.h>
#include <libdevcore/SHA3.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string/replace.hpp>

#include <utility>
#include <numeric>
```

See [this issue](http://stackoverflow.com/questions/614302/c-header-order/614333#614333 "C header order") for the reason: this makes it easier to find missing includes in header files.

## 13. Recommended reading

- Herb Sutter and Bjarne Stroustrup:
  - [C++ Core Guidelines](https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)

- Herb Sutter and Andrei Alexandrescu:
  - "C++ Coding Standards: 101 Rules, Guidelines, and Best Practices"

- Scott Meyers:
  - "Effective C++: 55 Specific Ways to Improve Your Programs and Designs (3rd Edition)"
  - "More Effective C++: 35 New Ways to Improve Your Programs and Designs"
  - "Effective Modern C++: 42 Specific Ways to Improve Your Use of C++11 and C++14"
