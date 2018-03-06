0. Formatting

GOLDEN RULE: Follow the style of the existing code when you make changes.

a. Use tabs for leading indentation
- tab stops are every 4 characters.
- One indentation level -> exactly one byte (i.e. a tab character) in the source file.
- If you have run-on lines, indent as you would for a block.
b. Line widths:
- Don't worry about having lines of code > 80-char wide.
- Lines of comments should be formatted according to ease of viewing, but simplicity is to be preferred over beauty.
c. Don't use braces for condition-body one-liners.
d. Never place condition bodies on same line as condition.
e. Space between first paren and keyword, but *not* following first paren or preceding final paren.
f. No spaces when fewer than intra-expression three parens together; when three or more, space according to clarity.
g. No spaces for subscripting or unary operators.
h. No space before ':' but one after it, except in the ternary operator: one on both sides.
i. Space all other operators.
j. Braces, when used, always have their own lines and are at same indentation level as "parent" scope.

(WRONG)
if( a==b[ i ] ) { printf ("Hello\n"); }
foo->bar(someLongVariableName,
         anotherLongVariableName,
         anotherLongVariableName,
         anotherLongVariableName,
         anotherLongVariableName);

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



1. Namespaces;

a. No "using namespace" declarations in header files.
b. All symbols should be declared in a namespace except for final applications.
c. Preprocessor symbols should be prefixed with the namespace in all-caps and an underscore.

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
b. Leading "c_" to const variables (unless part of an external API).
c. Leading "g_" to global (non-const) variables.
d. Leading "s_" to static (non-const, non-global) variables.



5. Error reporting:

- Prefer exception to bool/int return type.



6. Declarations:

a. {Typename} + {qualifiers} + {name}.
b. Only one per line.
c. Associate */& with type, not variable (at ends with parser, but more readable, and safe if in conjunction with (b)).
d. Favour declarations close to use; don't habitually declare at top of scope ala C.
e. Always pass non-trivial parameters with a const& suffix.
f. If a function returns multiple values, use std::tuple (std::pair acceptable). Prefer not using */& arguments, except where efficiency requires.
g. Never use a macro where adequate non-preprocessor C++ can be written.
h. Make use of auto whenever type is clear or unimportant:
- Always avoid doubly-stating the type.
- Use to avoid vast and unimportant type declarations.
- However, avoid using auto where type is not immediately obvious from the context, and especially not for arithmetic expressions.
i. Don't pass bools: prefer enumerations instead.
j. Prefer enum class to straight enum.


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
double const d = 0;
int i;
int j;
char* s;
std::tuple<float, float> meanAndSigma(std::vector<float> const& _v, Accuracy _a);
auto x = dynamic_cast<Derived*>(base);
for (auto i = x.begin(); i != x.end(); ++i) {}


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

a. Collection conventions:
- -s means std::vector e.g. using MyTypes = std::vector<MyType>
- -Set means std::set e.g. using MyTypeSet = std::set<MyType>
- -Hash means std::unordered_set e.g. using MyTypeHash = std::unordered_set<MyType>
b. Class conventions:
- -Face means the interface of some shared concept. (e.g. FooFace might be a pure virtual class.)
c. Avoid unpronouncable names;
- If you need to shorten a name favour a pronouncable slice of the original to a scattered set of consonants.
- e.g. Manager shortens to Man rather than Mgr.
d. Avoid prefixes of initials (e.g. DON'T use IMyInterface, CMyImplementation)
e. Find short, memorable & (at least semi-) descriptive names for commonly used classes or name-fragments.
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
- e.g. using Guard = std::lock_guard<std::mutex>; ///< Guard is used throughout the codebase since it's clear in meaning and used commonly. 
d. In general expressions should be roughly as important/semantically meaningful as the space they occupy.



11. Commenting

a. Comments should be doxygen-compilable, using @notation rather than \notation.
b. Document the interface, not the implementation.
- Documentation should be able to remain completely unchanged, even if the method is reimplemented.
- Comment in terms of the method properties and intended alteration to class state (or what aspects of the state it reports).
- Be careful to scrutinise documentation that extends only to intended purpose and usage.
- Reject documentation that is simply an English transaction of the implementation.



12. Include Headers

Includes should go in increasing order of generality (libethereum -> libethcore -> libdevcrypto -> libdevcore -> boost -> STL).  For example:

#include <libethereum/Defaults.h>
#include <libdevcrypto/SHA3.h>
#include <libdevcore/Log.h>
#include <libdevcore/Exceptions.h>
#include <libdevcore/CommonData.h>
#include <libdevcore/Common.h>
#include <boost/filesystem.hpp>
#include <string>

See http://stackoverflow.com/questions/614302/c-header-order/614333#614333 for the reason: this makes it easier to find missing includes in header files.



13. Logging

Logging should be performed at appropriate verbosities depending on the logging message.
The more likely a message is to repeat (and thus cause noise) the higher in verbosity it should be.
Some rules to keep in mind:

 - Verbosity == 0 -> Reserved for important stuff that users must see and can understand.
 - Verbosity == 1 -> Reserved for stuff that users don't need to see but can understand.
 - Verbosity >= 2 -> Anything that is or might be displayed more than once every minute
 - Verbosity >= 3 -> Anything that only a developer would understand
 - Verbosity >= 4 -> Anything that is low-level (e.g. peer disconnects, timers being cancelled)


14. Recommended reading

Herb Sutter and Bjarne Stroustrup
- "C++ Core Guidelines" (https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md)

Herb Sutter and Andrei Alexandrescu
- "C++ Coding Standards: 101 Rules, Guidelines, and Best Practices"

Scott Meyers
- "Effective C++: 55 Specific Ways to Improve Your Programs and Designs (3rd Edition)"
- "More Effective C++: 35 New Ways to Improve Your Programs and Designs"
- "Effective Modern C++: 42 Specific Ways to Improve Your Use of C++11 and C++14"
