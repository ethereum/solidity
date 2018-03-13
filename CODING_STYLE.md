0. Formatting

GOLDEN RULE: Follow the style of the existing code when you make changes.

a. Use tabs for leading indentation
- tab stops are every 4 characters (only relevant for line length).
- One indentation level -> exactly one byte (i.e. a tab character) in the source file.
b. Line widths:
- Lines should be at most 99 characters wide to make diff views readable and reduce merge conflicts.
- Lines of comments should be formatted according to ease of viewing, but simplicity is to be preferred over beauty.
c. Single-statement blocks should not have braces, unless required for clarity.
d. Never place condition bodies on same line as condition.
e. Space between keyword and opening parenthesis, but not following opening parenthesis or before final parenthesis.
f. No spaces for unary operators, `->` or `.`.
g. No space before ':' but one after it, except in the ternary operator: one on both sides.
h. Add spaces around all other operators.
i. Braces, when used, always have their own lines and are at same indentation level as "parent" scope.
j. If lines are broken, a list of elements enclosed with parentheses (of any kind) and separated by a
   separator (of any kind) are formatted such that there is exactly one element per line, followed by
   the separator, the opening parenthesis is on the first line, followed by a line break and the closing
   parenthesis is on a line of its own (unindented). See example below.

(WRONG)
if( a==b[ i ] ) { printf ("Hello\n"); }
foo->bar(someLongVariableName,
         anotherLongVariableName,
         anotherLongVariableName,
         anotherLongVariableName,
         anotherLongVariableName);
cout << "some very long string that contains completely irrelevant text that talks about this and that and contains the words \"lorem\" and \"ipsum\"" << endl;

(RIGHT)
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



1. Namespaces;

a. No "using namespace" declarations in header files.
b. All symbols should be declared in a namespace except for final applications.
c. Use anonymous namespaces for helpers whose scope is a cpp file only.
d. Preprocessor symbols should be prefixed with the namespace in all-caps and an underscore.

(WRONG)
#include <cassert>
using namespace std;
tuple<float, float> meanAndSigma(vector<float> const& _v);

(CORRECT)
#include <cassert>
std::tuple<float, float> meanAndSigma(std::vector<float> const& _v);



2. Preprocessor;

a. File comment is always at top, and includes:
- Copyright.
- License (e.g. see COPYING).
b. Never use #ifdef/#define/#endif file guards. Prefer #pragma once as first line below file comment.
c. Prefer static const variable to value macros.
d. Prefer inline constexpr functions to function macros.
e. Split complex macro on multiple lines with '\'.



3. Capitalization;

GOLDEN RULE: Preprocessor: ALL_CAPS; C++: camelCase.

a. Use camelCase for splitting words in names, except where obviously extending STL/boost functionality in which case follow those naming conventions.
b. The following entities' first alpha is upper case:
- Type names.
- Template parameters.
- Enum members.
- static const variables that form an external API.
c. All preprocessor symbols (macros, macro arguments) in full uppercase with underscore word separation.


All other entities' first alpha is lower case.



4. Variable prefixes:

a. Leading underscore "_" to parameter names.
- Exception: "o_parameterName" when it is used exclusively for output. See 6(f).
- Exception: "io_parameterName" when it is used for both input and output. See 6(f).
b. Leading "g_" to global (non-const) variables.
c. Leading "s_" to static (non-const, non-global) variables.



5. Assertions:

- use `solAssert` and `solUnimplementedAssert` generously to check assumptions
  that span across different parts of the code base, for example before dereferencing
  a pointer.


6. Declarations:

a. {Typename} + {qualifiers} + {name}.
b. Only one per line.
c. Associate */& with type, not variable (at ends with parser, but more readable, and safe if in conjunction with (b)).
d. Favour declarations close to use; don't habitually declare at top of scope ala C.
e. Pass non-trivial parameters as const reference, unless the data is to be copied into the function, then either pass by const reference or by value and use std::move.
f. If a function returns multiple values, use std::tuple (std::pair acceptable) or better introduce a struct type. Do not use */& arguments.
g. Use parameters of pointer type only if ``nullptr`` is a valid argument, use references otherwise. Often, ``boost::optional`` is better suited than a raw pointer.
h. Never use a macro where adequate non-preprocessor C++ can be written.
i. Only use ``auto`` if the type is very long and rather irrelevant.
j. Do not pass bools: prefer enumerations instead.
k. Prefer enum class to straight enum.
l. Always initialize POD variables, even if their value is overwritten later.


(WRONG)
const double d = 0;
int i, j;
char *s;
float meanAndSigma(std::vector<float> _v, float* _sigma, bool _approximate);
Derived* x(dynamic_cast<Derived*>(base));
for (map<ComplexTypeOne, ComplexTypeTwo>::iterator i = l.begin(); i != l.end(); ++l) {}


(CORRECT)
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


7. Structs & classes

a. Structs to be used when all members public and no virtual functions.
- In this case, members should be named naturally and not prefixed with 'm_'
b. Classes to be used in all other circumstances.



8. Members:

a. One member per line only.
b. Private, non-static, non-const fields prefixed with m_.
c. Avoid public fields, except in structs.
d. Use override, final and const as much as possible.
e. No implementations with the class declaration, except:
- template or force-inline method (though prefer implementation at bottom of header file).
- one-line implementation (in which case include it in same line as declaration).
f. For a property 'foo'
- Member: m_foo;
- Getter: foo() [ also: for booleans, isFoo() ];
- Setter: setFoo();



9. Naming

a. Avoid unpronouncable names
b. Names should be shortened only if they are extremely common, but shortening should be generally avoided
c. Avoid prefixes of initials (e.g. do not use IMyInterface, CMyImplementation)
c. Find short, memorable & (at least semi-) descriptive names for commonly used classes or name-fragments.
- A dictionary and thesaurus are your friends.
- Spell correctly.
- Think carefully about the class's purpose.
- Imagine it as an isolated component to try to decontextualise it when considering its name.
- Don't be trapped into naming it (purely) in terms of its implementation.



10. Type-definitions

a. Prefer 'using' to 'typedef'. e.g. using ints = std::vector<int>; rather than typedef std::vector<int> ints;
b. Generally avoid shortening a standard form that already includes all important information:
- e.g. stick to shared_ptr<X> rather than shortening to ptr<X>.
c. Where there are exceptions to this (due to excessive use and clear meaning), note the change prominently and use it consistently.
- e.g. using Guard = std::lock_guard<std::mutex>; ///< Guard is used throughout the codebase since it is clear in meaning and used commonly. 
d. In general expressions should be roughly as important/semantically meaningful as the space they occupy.
e. Avoid introducing aliases for types unless they are very complicated. Consider the number of items a brain can keep track of at the same time.



11. Commenting

a. Comments should be doxygen-compilable, using @notation rather than \notation.
b. Document the interface, not the implementation.
- Documentation should be able to remain completely unchanged, even if the method is reimplemented.
- Comment in terms of the method properties and intended alteration to class state (or what aspects of the state it reports).
- Be careful to scrutinise documentation that extends only to intended purpose and usage.
- Reject documentation that is simply an English transaction of the implementation.
c. Avoid in-code comments. Instead, try to extract blocks of functionality into functions. This often already eliminates the need for an in-code comment.


12. Include Headers

Includes should go in increasing order of generality (libsolidity -> libevmasm -> libdevcore -> boost -> STL).
The corresponding .h file should be the first include in the respective .cpp file.
Insert empty lines between blocks of include files.

Example:

```
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

See http://stackoverflow.com/questions/614302/c-header-order/614333#614333 for the reason: this makes it easier to find missing includes in header files.


13. Recommended reading

Herb Sutter and Bjarne Stroustrup
- "C++ Core Guidelines" (https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)

Herb Sutter and Andrei Alexandrescu
- "C++ Coding Standards: 101 Rules, Guidelines, and Best Practices"

Scott Meyers
- "Effective C++: 55 Specific Ways to Improve Your Programs and Designs (3rd Edition)"
- "More Effective C++: 35 New Ways to Improve Your Programs and Designs"
- "Effective Modern C++: 42 Specific Ways to Improve Your Use of C++11 and C++14"
