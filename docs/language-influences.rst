###################
Language Influences
###################

Solidity is a `curly-bracket language <https://en.wikipedia.org/wiki/List_of_programming_languages_by_type#Curly-bracket_languages>`_
that has been influenced and inspired by several well-known programming languages.

Solidity is most profoundly influenced by C++, but also borrowed concepts from languages like
Python, JavaScript, and others.

The influence from C++ can be seen in the syntax for variable declarations, for loops, the concept
of overloading functions, implicit and explicit type conversions and many other details.

In the early days of the language, Solidity used to be partly influenced by JavaScript.
This was due to function-level scoping of variables and the use of the keyword ``var``.
The JavaScript influence was reduced starting from version 0.4.0.
Now, the main remaining similarity to JavaScript is that functions are defined using the keyword
``function``. Solidity also supports import syntax and semantics that
are similar to those available in JavaScript. Besides those points, Solidity looks like
most other curly-bracket languages and has no major JavaScript influence anymore.

Another influence to Solidity was Python. Solidity's modifiers were added trying to model
Python's decorators with a much more restricted functionality. Furthermore, multiple inheritance, C3 linearization,
and the ``super`` keyword are taken from Python as well as the general assignment and copy semantics of value
and reference types.
