********************************
Layout of a Solidity Source File
********************************

Source files can contain an arbitrary number of
:ref:`contract definitions<contract_structure>`, import_ directives
and :ref:`pragma directives<pragma>`.

.. index:: ! pragma

.. _pragma:

Pragmas
=======

The ``pragma`` keyword is used to enable certain compiler features
or checks. A pragma directive is always local to a source file, so
you have to add the pragma to all your files if you want enable it
in all of your project. If you :ref:`import<import>` another file, the pragma
from that file does not automatically apply to the importing file.

.. index:: ! pragma, version

.. _version_pragma:

Version Pragma
--------------

Source files can (and should) be annotated with a version pragma to reject
compilation with future compiler versions that might introduce incompatible
changes. We try to keep these to an absolute minimum and
introduce them in a way that changes in semantics also require changes
in the syntax, but this is not always possible. Because of this, it is always
a good idea to read through the changelog at least for releases that contain
breaking changes. These releases always have versions of the form
``0.x.0`` or ``x.0.0``.

The version pragma is used as follows::

  pragma solidity ^0.5.2;

A source file with the line above does not compile with a compiler earlier than version 0.5.2,
and it also does not work on a compiler starting from version 0.6.0 (this
second condition is added by using ``^``). This is because
there will be no breaking changes until version ``0.6.0``, so you can always
be sure that your code compiles the way you intended. The exact version of the
compiler is not fixed, so that bugfix releases are still possible.

It is possible to specify more complex rules for the compiler version,
these follow the same syntax used by `npm <https://docs.npmjs.com/misc/semver>`_.

.. note::
  Using the version pragma *does not* change the version of the compiler.
  It also *does not* enable or disable features of the compiler. It just
  instructs the compiler to check whether its version matches the one
  required by the pragma. If it does not match, the compiler issues
  an error.

.. index:: ! pragma, experimental

.. _experimental_pragma:

Experimental Pragma
-------------------

The second pragma is the experimental pragma. It can be used to enable
features of the compiler or language that are not yet enabled by default.
The following experimental pragmas are currently supported:


ABIEncoderV2
~~~~~~~~~~~~

The new ABI encoder is able to encode and decode arbitrarily nested
arrays and structs. It produces less optimal code (the optimizer
for this part of the code is still under development) and has not
received as much testing as the old encoder. You can activate it
using ``pragma experimental ABIEncoderV2;``.

.. _smt_checker:

SMTChecker
~~~~~~~~~~

This component has to be enabled when the Solidity compiler is built
and therefore it is not available in all Solidity binaries.
The :ref:`build instructions<smt_solvers_build>` explain how to activate this option.
It is activated for the Ubuntu PPA releases in most versions,
but not for solc-js, the Docker images, Windows binaries or the
statically-built Linux binaries.

If you use ``pragma experimental SMTChecker;``, then you get additional
:ref:`safety warnings<formal_verification>` which are obtained by querying an
SMT solver.
The component does not yet support all features of the Solidity language and
likely outputs many warnings. In case it reports unsupported features, the
analysis may not be fully sound.

.. index:: source file, ! import, module

.. _import:

Importing other Source Files
============================

Syntax and Semantics
--------------------

Solidity supports import statements to help modularise your code that are similar to those available in JavaScript
(from ES6 on). However, Solidity does not support the concept of a `default export <https://developer.mozilla.org/en-US/docs/web/javascript/reference/statements/export#Description>`_.

At a global level, you can use import statements of the following form:

::

  import "filename";

This statement imports all global symbols from "filename" (and symbols imported there) into the
current global scope (different than in ES6 but backwards-compatible for Solidity).
This form is not recommended for use, because it unpredictably pollutes the namespace.
If you add new top-level items inside "filename", they automatically
appear in all files that import like this from "filename". It is better to import specific
symbols explicitly.

The following example creates a new global symbol ``symbolName`` whose members are all
the global symbols from ``"filename"``:

::

  import * as symbolName from "filename";

which results in all global symbols being available in the format ``symbolName.symbol``.

A variant of this syntax that is not part of ES6, but possibly useful is:

::

  import "filename" as symbolName;

which is equivalent to ``import * as symbolName from "filename";``.

If there is a naming collision, you can rename symbols while importing. For example,
the code below creates new global symbols ``alias`` and ``symbol2`` which reference
``symbol1`` and ``symbol2`` from inside ``"filename"``, respectively.

::

  import {symbol1 as alias, symbol2} from "filename";

Paths
-----

In the above, ``filename`` is always treated as a path with ``/`` as directory separator,
and ``.`` as the current and ``..`` as the parent directory.  When ``.`` or ``..`` is followed by a character except ``/``,
it is not considered as the current or the parent directory.
All path names are treated as absolute paths unless they start with the current ``.`` or the parent directory ``..``.

To import a file ``filename`` from the same directory as the current file, use ``import "./filename" as symbolName;``.
If you use ``import "filename" as symbolName;`` instead, a different file could be referenced
(in a global "include directory").

It depends on the compiler (see :ref:`import-compiler`) how to actually resolve the paths.
In general, the directory hierarchy does not need to strictly map onto your local
filesystem, and the path can also map to resources such as ipfs, http or git.

.. note::
    Always use relative imports like ``import "./filename.sol";`` and avoid
    using ``..`` in path specifiers. In the latter case, it is probably better to use
    global paths and set up remappings as explained below.

.. _import-compiler:

Use in Actual Compilers
-----------------------

When invoking the compiler, you can specify how to discover the first element
of a path, and also path prefix remappings. For
example you can setup a remapping so that everything imported from the virtual
directory ``github.com/ethereum/dapp-bin/library`` would actually be read from
your local directory ``/usr/local/dapp-bin/library``.
If multiple remappings apply, the one with the longest key is tried first.
An empty prefix is not allowed. The remappings can depend on a context,
which allows you to configure packages to import e.g., different versions of a
library of the same name.

**solc**:

For solc (the commandline compiler), you provide these path remappings as
``context:prefix=target`` arguments, where both the ``context:`` and the
``=target`` parts are optional (``target`` defaults to ``prefix`` in this
case). All remapping values that are regular files are compiled (including
their dependencies).

This mechanism is backwards-compatible (as long
as no filename contains ``=`` or ``:``) and thus not a breaking change. All
files in or below the ``context`` directory that import a file that starts with
``prefix`` are redirected by replacing ``prefix`` by ``target``.

For example, if you clone ``github.com/ethereum/dapp-bin/`` locally to
``/usr/local/dapp-bin``, you can use the following in your source file:

::

  import "github.com/ethereum/dapp-bin/library/iterable_mapping.sol" as it_mapping;

Then run the compiler:

.. code-block:: bash

  solc github.com/ethereum/dapp-bin/=/usr/local/dapp-bin/ source.sol

As a more complex example, suppose you rely on a module that uses an old
version of dapp-bin that you checked out to ``/usr/local/dapp-bin_old``, then you can run:

.. code-block:: bash

  solc module1:github.com/ethereum/dapp-bin/=/usr/local/dapp-bin/ \
       module2:github.com/ethereum/dapp-bin/=/usr/local/dapp-bin_old/ \
       source.sol

This means that all imports in ``module2`` point to the old version but imports
in ``module1`` point to the new version.

.. note::

  ``solc`` only allows you to include files from certain directories. They have
  to be in the directory (or subdirectory) of one of the explicitly specified
  source files or in the directory (or subdirectory) of a remapping target. If
  you want to allow direct absolute includes, add the remapping ``/=/``.

If there are multiple remappings that lead to a valid file, the remapping
with the longest common prefix is chosen.

**Remix**:

`Remix <https://remix.ethereum.org/>`_ provides an automatic remapping for
GitHub and automatically retrieves the file over the network. You can import
the iterable mapping as above,  e.g.

::
  import "github.com/ethereum/dapp-bin/library/iterable_mapping.sol" as it_mapping;

Remix may add other source code providers in the future.

.. index:: ! comment, natspec

Comments
========

Single-line comments (``//``) and multi-line comments (``/*...*/``) are possible.

::

  // This is a single-line comment.

  /*
  This is a
  multi-line comment.
  */

.. note::
  A single-line comment is terminated by any unicode line terminator
  (LF, VF, FF, CR, NEL, LS or PS) in utf8 encoding. The terminator is still part of
  the source code after the comment, so if it is not an ascii symbol
  (these are NEL, LS and PS), it will lead to a parser error.

Additionally, there is another type of comment called a natspec comment,
which is detailed in the :ref:`style guide<natspec>`. They are written with a
triple slash (``///``) or a double asterisk block(``/** ... */``) and
they should be used directly above function declarations or statements.
You can use `Doxygen <https://en.wikipedia.org/wiki/Doxygen>`_-style tags inside these comments to document
functions, annotate conditions for formal verification, and provide a
**confirmation text** which is shown to users when they attempt to invoke a
function.

In the following example we document the title of the contract, the explanation
for the two function parameters and two return variables.

::

    pragma solidity >=0.4.0 <0.7.0;

    /** @title Shape calculator. */
    contract ShapeCalculator {
        /** @dev Calculates a rectangle's surface and perimeter.
          * @param w Width of the rectangle.
          * @param h Height of the rectangle.
          * @return s The calculated surface.
          * @return p The calculated perimeter.
          */
        function rectangle(uint w, uint h) public pure returns (uint s, uint p) {
            s = w * h;
            p = 2 * (w + h);
        }
    }
