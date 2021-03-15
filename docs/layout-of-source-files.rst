********************************
Layout of a Solidity Source File
********************************

Source files can contain an arbitrary number of
:ref:`contract definitions<contract_structure>`, import_ directives,
:ref:`pragma directives<pragma>` and
:ref:`struct<structs>`, :ref:`enum<enums>`, :ref:`function<functions>`, :ref:`error<errors>`
and :ref:`constant variable<constants>` definitions.

.. index:: ! license, spdx

SPDX License Identifier
=======================

Trust in smart contract can be better established if their source code
is available. Since making source code available always touches on legal problems
with regards to copyright, the Solidity compiler encourages the use
of machine-readable `SPDX license identifiers <https://spdx.org>`_.
Every source file should start with a comment indicating its license:

``// SPDX-License-Identifier: MIT``

The compiler does not validate that the license is part of the
`list allowed by SPDX <https://spdx.org/licenses/>`_, but
it does include the supplied string in the :ref:`bytecode metadata <metadata>`.

If you do not want to specify a license or if the source code is
not open-source, please use the special value ``UNLICENSED``.

Supplying this comment of course does not free you from other
obligations related to licensing like having to mention
a specific license header in each source file or the
original copyright holder.

The comment is recognized by the compiler anywhere in the file at the
file level, but it is recommended to put it at the top of the file.

More information about how to use SPDX license identifiers
can be found at the `SPDX website <https://spdx.org/ids-how>`_.


.. index:: ! pragma

.. _pragma:

Pragmas
=======

The ``pragma`` keyword is used to enable certain compiler features
or checks. A pragma directive is always local to a source file, so
you have to add the pragma to all your files if you want to enable it
in your whole project. If you :ref:`import<import>` another file, the pragma
from that file does *not* automatically apply to the importing file.

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

The version pragma is used as follows: ``pragma solidity ^0.5.2;``

A source file with the line above does not compile with a compiler earlier than version 0.5.2,
and it also does not work on a compiler starting from version 0.6.0 (this
second condition is added by using ``^``). Because
there will be no breaking changes until version ``0.6.0``, you can
be sure that your code compiles the way you intended. The exact version of the
compiler is not fixed, so that bugfix releases are still possible.

It is possible to specify more complex rules for the compiler version,
these follow the same syntax used by `npm <https://docs.npmjs.com/cli/v6/using-npm/semver>`_.

.. note::
  Using the version pragma *does not* change the version of the compiler.
  It also *does not* enable or disable features of the compiler. It just
  instructs the compiler to check whether its version matches the one
  required by the pragma. If it does not match, the compiler issues
  an error.

ABI Coder Pragma
----------------

By using ``pragma abicoder v1`` or ``pragma abicoder v2`` you can
select between the two implementations of the ABI encoder and decoder.

The new ABI coder (v2) is able to encode and decode arbitrarily nested
arrays and structs. It might produce less optimal code and has not
received as much testing as the old encoder, but is considered
non-experimental as of Solidity 0.6.0. You still have to explicitly
activate it using ``pragma abicoder v2;``. Since it will be
activated by default starting from Solidity 0.8.0, there is the option to select
the old coder using ``pragma abicoder v1;``.

The set of types supported by the new encoder is a strict superset of
the ones supported by the old one. Contracts that use it can interact with ones
that do not without limitations. The reverse is possible only as long as the
non-``abicoder v2`` contract does not try to make calls that would require
decoding types only supported by the new encoder. The compiler can detect this
and will issue an error. Simply enabling ``abicoder v2`` for your contract is
enough to make the error go away.

.. note::
  This pragma applies to all the code defined in the file where it is activated,
  regardless of where that code ends up eventually. This means that a contract
  whose source file is selected to compile with ABI coder v1
  can still contain code that uses the new encoder
  by inheriting it from another contract. This is allowed if the new types are only
  used internally and not in external function signatures.

.. note::
  Up to Solidity 0.7.4, it was possible to select the ABI coder v2
  by using ``pragma experimental ABIEncoderV2``, but it was not possible
  to explicitly select coder v1 because it was the default.

.. index:: ! pragma, experimental

.. _experimental_pragma:

Experimental Pragma
-------------------

The second pragma is the experimental pragma. It can be used to enable
features of the compiler or language that are not yet enabled by default.
The following experimental pragmas are currently supported:


ABIEncoderV2
~~~~~~~~~~~~

Because the ABI coder v2 is not considered experimental anymore,
it can be selected via ``pragma abicoder v2`` (please see above)
since Solidity 0.7.4.

.. _smt_checker:

SMTChecker
~~~~~~~~~~

This component has to be enabled when the Solidity compiler is built
and therefore it is not available in all Solidity binaries.
The :ref:`build instructions<smt_solvers_build>` explain how to activate this option.
It is activated for the Ubuntu PPA releases in most versions,
but not for the Docker images, Windows binaries or the
statically-built Linux binaries. It can be activated for solc-js via the
`smtCallback <https://github.com/ethereum/solc-js#example-usage-with-smtsolver-callback>`_ if you have an SMT solver
installed locally and run solc-js via node (not via the browser).

If you use ``pragma experimental SMTChecker;``, then you get additional
:ref:`safety warnings<formal_verification>` which are obtained by querying an
SMT solver.
The component does not yet support all features of the Solidity language and
likely outputs many warnings. In case it reports unsupported features, the
analysis may not be fully sound.

.. index:: source file, ! import, module, source unit

.. _import:

Importing other Source Files
============================

Syntax and Semantics
--------------------

Solidity supports import statements to help modularise your code that
are similar to those available in JavaScript
(from ES6 on). However, Solidity does not support the concept of
a `default export <https://developer.mozilla.org/en-US/docs/web/javascript/reference/statements/export#Description>`_.

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

.. _path-resolution:

Path Resolution
---------------

The paths used in imports in a general case do not have to be filesystem paths.
In the simplest cases they may end up being used directly to load files from disk but in general
the final path can be substantially different due to abstractions necessary to ensure reproducible
builds on different platforms.

.. index:: ! virtual filesystem, ! source unit ID, ! import; path, filesystem path
.. _virtual-filesystem:

Virtual Filesystem
~~~~~~~~~~~~~~~~~~

The compiler maintains an internal database (virtual filesystem) where each compiled source unit is
assigned a unique *source unit ID* which is an opaque and unstructured identifier.

While a source unit ID in the virtual filesystem can be anything, it should be a valid path if the
source is meant to be loaded from the underlying filesystem.
When the requested ID is not present in the virtual filesystem, it is passed to the file loader.
In case of the command-line compiler the file loader simply uses it as a path.
The `JavaScript interface <https://github.com/ethereum/solc-js>`_ is a bit more flexible in that
regard and allows the user to provide a callback to perform this operation - in this case the
IDs can be arbitrary.
For example they could be URLs as long as the custom loader can handle them.

There are several ways to load source units into the virtual filesystem:

#. **import statement**

   The ``import`` statement requests a module from the compiler and allows to access certain symbols
   from that module.

   We will refer to the path used in the statement as *import path*.
   The import path is translated into a source unit ID first and then the compiler uses the ID to
   look up the file in its virtual filesystem.

   Imports can be broadly classified into three categories based on how the path is specified:

   .. code-block:: solidity

       import "/contracts/lib/token.sol";   // Absolute
       import "contracts/lib/token.sol";    // Relative to base
       import "./contracts/lib/token.sol";  // Relative to source
       import "../contracts/lib/token.sol"; // Relative to source

   There is actually no distinction between :ref:`absolute imports <absolute-imports>` and
   :ref:`imports relative to base <imports-relative-to-base>` at the virtual filesystem level.
   In both cases the import path is translated into a source unit ID using the same rules and they
   are only handled differently by the default file loader.

   :ref:`Imports relative to source <imports-relative-to-source>`, on the other hand, need to be
   interpreted as paths and normalized to be properly resolved into source unit IDs.
   The path in this case must conform to UNIX path conventions regardless of the underlying platform.

   .. _virtual-filesystem-loading-files-cli:

#. **CLI**

   To compile a file using the command-line interface of the compiler you specify one or more paths:

   .. code-block:: bash

       solc contract.sol /usr/local/dapp-bin/token.sol

   These are interpreted as *filesystem paths* and the rules for translating them into source unit IDs
   are different than for import paths.
   Most importantly, filesystem paths are platform-specific while import paths are not.
   For example a path like ``C:\project\contract.sol`` will be interpreted differently on Windows
   and on systems that follow the UNIX path conventions.

   It does not matter if the path you specify is relative or absolute.
   If the path is relative, it is converted into an absolute one by prepending the current working
   directory.
   Then the path is normalized, which involves first a conversion from the platform-specific format
   the internal UNIX-like format, collapsing all the relative ``./`` and ``../`` segments and
   removing redundant slashes.
   Finally, :ref:`the base path <imports-relative-to-base>` is stripped from the source unit ID.
   This way the resulting ID is a path relative to base if and only if the file is located inside
   the base directory.

#. **Standard JSON (as content)**

   An alternative way to compile your project is to use the ``--standard-json`` option and provide
   a JSON file containing all of your source code:

   .. code-block:: json

       {
           "language": "Solidity",
           "sources": {
               "contract.sol": {
                   "content": "import \"./util.sol\";\ncontract C {}"
               },
               "util.sol": {
                   "content": "library Util {}"
               },
               "/usr/local/dapp-bin/token.sol": {
                   "content": "contract Token {}"
               }
           },
           "settings": {"outputSelection": {"*": { "*": ["metadata", "evm.bytecode"]}}}
       }

   The ``sources`` dictionary specifies the initial content of the virtual filesystem and you
   can use source unit IDs directly there.
   They do not undergo any extra translation or normalization.

   The path to the JSON file does not affect the path resolution in any way.
   In fact, it is common to supply it on the standard input in which case it does not have a path at all.

   .. note::

       When using ``--standard-json`` you cannot provide additional source files as command-line
       arguments but it does not mean that the compiler will not load any extra files from disk.
       If a contract imports a file that is not present in ``sources``, the compiler will use the
       file loader as in any other situation, which may result in the source being read from disk
       (or provided by the callback when using the JavaScript interface).

#. **Standard JSON (as URL)**

   When using Standard JSON it is possible to tell the compiler to load the files from disk directly:

   .. code-block:: json

       {
           "language": "Solidity",
           "sources": {
               "/usr/local/dapp-bin/token.sol": {
                   "urls": ["/projects/mytoken.sol"]
               }
           },
           "settings": {"outputSelection": {"*": { "*": ["metadata", "evm.bytecode"]}}}
       }

   The path specified in ``urls`` is only passed to the file loader and used to locate the file.
   It does not affect the source unit ID and is not included in contract metadata.

   Paths in ``urls`` are affected by base path and any other transformations performed by the file loader.

#. **Standard input**

   The last way to provide the source is by sending it to compiler's standard input:

   .. code-block:: bash

       echo 'import "./util.sol"; contract C {}' | solc -

   The content of the standard input is identified in the virtual filesystem by a special source unit ID:
   ``<stdin>``.

.. warning::

    The compiler uses source unit IDs to determine whether imports refer to the same source unit or not.
    If you refer to a file in multiple ways that translate to different IDs, it will be compiled
    multiple times.

    For example:

    .. code-block:: solidity
        :caption: /code/contract.sol

        import "tokens/token.sol" as token1;   // source unit ID: tokens/token.sol
        import "tokens///token.sol" as token2; // source unit ID: tokens///token.sol

    .. code-block:: bash

        cd /code
        solc contract.sol /code/tokens/token.sol # source unit ID: /code/tokens/token.sol

    In the above ``token.sol`` will end up in the virtual filesystem under three different
    source unit IDs even though all the paths refer to the same file in the underlying filesystem.

    To avoid this situation it is recommended to always use the canonical form of paths in your
    imports and to only list the top-level files that are not imported by other files when
    invoking the CLI compiler.

Now that we know how the virtual filesystem works, let us go through the rules used to translate
import paths into source unit IDs in more detail.

.. index:: absolute import
.. _absolute-imports:

Absolute Imports
~~~~~~~~~~~~~~~~

An *absolute import* always starts with a forward slash (``/``).
The import path translates directly to a source unit ID without normalization of any kind:

::

    import "/project/lib/util.sol" as util;          // source unit ID: /project/lib/util.sol
    import "/project/lib/../lib///math.sol" as math; // source unit ID: /project/lib/../lib///math.sol

In the above you might expect the source unit ID be reduced to ``/project/lib/math.sol`` but it is
in fact ``/project/lib/../lib///math.sol``, exactly as stated in the file.

If no file is present under that ID in the virtual filesystem, the file loader will also use it as
is for filesystem lookup.
The resulting filesystem path is not affected by the value of base path.

.. index:: import relative to base, relative import
.. _imports-relative-to-base:

Imports Relative to Base
~~~~~~~~~~~~~~~~~~~~~~~~

Any import that does not start with ``/``, ``./`` or ``../`` is an *import relative to base*.

::

    import "lib/util.sol" as util;                   // source unit ID: lib/util.sol
    import "@openzeppelin/address.sol" as address;   // source unit ID: @openzeppelin/address.sol
    import "https://example.com/token.sol" as token; // source unit ID: https://example.com/token.sol

There is no difference between such imports and absolute ones at the virtual filesystem level.
The compiler sees both as opaque identifiers and there is no normalization involved:

::

    import "lib/../lib///math.sol" as math; // source unit ID: lib/../lib///math.sol

Only when the ID is passed to the file loader and needs to be converted into an actual filesystem
path different rules kick in.
To convert the path into an absolute one, the loader combines it with the path specified using the
``--base-path`` option.
If the base path itself is relative, it is interpreted as relative to the current working directory
just like any other path given on the command line.

Base path also affects :ref:`the way paths specified on the command line are converted into source
IDs <virtual-filesystem-loading-files-cli>`.
The source unit ID normally is the absolute, normalized path to the file in the UNIX format but if
the file happens to be inside the directory designated as the base path or one of its subdirectories
the prefix is stripped from its source unit ID and it becomes relative to base.

.. code-block:: bash

    cd /home/user
    solc /project/contract.sol                      # source unit ID: /project/contract.sol
    solc /project/contract.sol --base-path /project # source unit ID: contract.sol

Note that if you do not specify base path, it is by default equal to the current working directory:

.. code-block:: bash

    cd /project
    solc /home/user/contract.sol                      # source unit ID: contract.sol
    solc /home/user/contract.sol --base-path /project # source unit ID: contract.sol

.. index:: import relative to source, relative import
.. _imports-relative-to-source:

Imports Relative to Source
~~~~~~~~~~~~~~~~~~~~~~~~~~

An import starting with ``./`` or ``../`` is *relative to source*.
It differs from imports relative to base in that the compiler does interpret it as a path and
combines it with the path of the importing source unit to get the source unit ID.

.. code-block:: solidity
    :caption: /project/lib/math.sol

    import "./util.sol" as util;    // source unit ID: /project/lib/util.sol
    import "../token.sol" as token; // source unit ID: /lib/token.sol

If the parent source unit ID is relative to base, the resulting source unit ID is relative to
base as well:

.. code-block:: solidity
    :caption: lib/math.sol

    import "./util.sol" as util;    // source unit ID: lib/util.sol
    import "../token.sol" as token; // source unit ID: token.sol

To evaluate the prefix, the compiler starts with the source unit ID of the importing source unit and
first strips the file name.
Then, for every ``../`` segment in the import path it strips one segment from the ID.

.. code-block:: solidity
    :caption: /a/b/c/contract.sol

    import "../util.sol";          // source unit ID: /a/b/util.sol
    import "../../util.sol";       // source unit ID: /a/util.sol
    import "../../../util.sol";    // source unit ID: /util.sol

    import "../././.././util.sol"; // source unit ID: /a/util.sol

If there are more ``../`` segments than directory segments in the parent source unit ID, the
evaluation stops at the root:

.. code-block:: solidity
    :caption: /a/b/c/contract.sol

    import "../../../../util.sol";       // source unit ID: /util.sol
    import "../../../../../util.sol";    // source unit ID: /util.sol
    import "../../../../../../util.sol"; // source unit ID: /util.sol

.. code-block:: solidity
    :caption: a/b/c/contract.sol

    import "../../../../util.sol";       // source unit ID: util.sol
    import "../../../../../util.sol";    // source unit ID: util.sol
    import "../../../../../../util.sol"; // source unit ID: util.sol

After stripping the leading relative segments, the import path is normalized so that the
resulting source unit ID does not contain any ``./`` or ``../``:

.. code-block:: solidity
    :caption: /a/b/c/contract.sol

    import "../../d/e///.././util.sol"; // source unit ID: /a/e/util.sol

This is quite different from imports relative to base where the ``///.././`` part would remain
in the source unit ID.

Note that the parent source unit ID is **not** normalized, and the ``./`` and ``../`` segments in it
have no special meaning:

.. code-block:: solidity
    :caption: ../lib/math.sol

    import "./util.sol" as util;    // source unit ID: ../lib/util.sol
    import "../token.sol" as token; // source unit ID: ../../token.sol

This may lead to surprising results in corner cases:

.. code-block:: solidity
    :caption: /a/./b/contract.sol

    import "../c/util.sol";       // source unit ID: /a/./c/util.sol
    import "../../c/util.sol";    // source unit ID: /a/c/util.sol
    import "../../../c/util.sol"; // source unit ID: /c/util.sol

.. note::

    The use of relative imports containing leading ``../`` segments is not recommended.
    The same effect can be achieved in a more reliable way by using either absolute imports with
    import remapping or imports relative to base.

.. index:: remapping, import remapping
.. _import-remapping:

Import Remapping
~~~~~~~~~~~~~~~~

Base path and relative imports on their own allow you to freely move your project around the
filesystem but force you to keep all files within a single directory and its subdirectories.
When using external libraries it is often desirable to keep their files in a separate location.
To help with that, the compiler provides another mechanism: import remapping.

Remapping allows you to use placeholders for source unit ID prefixes and then have the compiler
replace them with actual paths.
For example you can set up a remapping so that everything imported from the virtual directory
``github.com/ethereum/dapp-bin/library`` would actually receive source unit IDs starting with
``dapp-bin/library``.
By setting base path to ``/project`` you could then have the compiler find them in
``/project/dapp-bin/library``

The remappings can depend on a context, which allows you to configure packages to import,
e.g. different versions of a library of the same name.

.. warning::

    Information about used remappings is stored in contract metadata so, while they let you avoid
    changing the source, they cannot be used to achieve reproducible builds.
    The metadata hash embedded in the bytecode will not be the same if you perform import remapping.

Path remappings have the form of ``context:prefix=target``.
All files in or below the ``context`` directory that import a file that starts with ``prefix`` are
redirected by replacing ``prefix`` with ``target``.
For example, if you clone ``github.com/ethereum/dapp-bin/`` locally to ``/project/dapp-bin``,
you can use the following in your source file:

::

    import "github.com/ethereum/dapp-bin/library/iterable_mapping.sol" as it_mapping;

Then run the compiler:

.. code-block:: bash

    solc github.com/ethereum/dapp-bin/=dapp-bin/ --base-path /project source.sol

As a more complex example, suppose you rely on a module that uses an old version of dapp-bin that
you checked out to ``/project/dapp-bin_old``, then you can run:

.. code-block:: bash

    solc module1:github.com/ethereum/dapp-bin/=dapp-bin/ \
         module2:github.com/ethereum/dapp-bin/=dapp-bin_old/ \
         --base-path /project \
         source.sol

This means that all imports in ``module2`` point to the old version but imports in ``module1``
point to the new version.

Here are the detailed rules governing the behaviour of remappings:

#. **Remappings only affect the translation between import paths and source unit IDs.**

   Source unit IDs added via other means cannot be remapped.
   For example the paths you specify on the command-line and the ones in ``sources.urls`` in
   Standard JSON are not affected.

    .. code-block:: bash

        solc /project=/contracts /project/contract.sol # source unit ID: /project/contract.sol

#. **Context and prefix must match source unit IDs, not import paths.**

   - This means that you cannot remap ``./`` or ``../`` directly since they are replaced during
     translation to source unit IDs but you can remap the source locations they resolve into:

     .. code-block:: bash

         solc ./=a /project=b /project/contract.sol

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "./util.sol" as util; // source unit ID: b/util.sol

   - You cannot remap base path or any other part of the path that is only added when the file is
     looked up in the underlying filesystem by the file loader:

     .. code-block:: bash

         solc /project=/contracts /project/contract.sol --base-path /project

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "util.sol" as util; // source unit ID: util.sol

#. **Target is inserted directly into the source unit ID and does not necessarily have to be a valid path.**

   - It can be anything as long as the file loader can handle it.
     In case of the command-line interface this includes also relative paths.
     When using the JavaScript interface you can just as well use URLs and abstract identifiers if
     your callback can handle them.

   - Remapping happens after paths relative to the source directory have already been resolved.
     This means that targets starting with ``./`` and ``../`` have no special meaning and are
     relative to the base directory rather than to the source location.

   - Remapping targets are not normalized so ``@root=./a/b//`` will remap ``@root/contract.sol``
     to ``./a/b//contract.sol`` and not ``a/b/contract.sol``.

   - If the target does not end with a slash, the compiler will not add one automatically:

     .. code-block:: bash

         solc /project/=/contracts /project/contract.sol

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "/project/util.sol" as util; // source unit ID: /contractsutil.sol

#. **Context and prefix are patterns and matches must be exact.**

   - ``a//b=c`` will not match ``a/b``.

   - Source unit IDs are not normalized so ``a/b=c`` will not match ``a//b`` either.

   - Parts of file and directory names can match as well.
     ``/newProject/con:/new=old`` will match ``/newProject/contract.sol`` and remap it to
     ``oldProject/contract.sol``.

#. **At most one remapping can be applied to a single import.**

   - If multiple remappings match the same source unit ID, the one with the longest matching
     prefix is chosen.
   - If prefixes are identical, the one specified last wins.
   - Remappings do not work on other remappings. For example ``a=b b=c c=d`` will not result in ``a``
     being remapped to ``d``.

#. **Prefix cannot be empty but context and target are optional.**

   If ``target`` is omitted, it defaults to the value of the ``prefix``.

.. note::

    ``solc`` only allows you to include files from certain directories.
    They have to be in the directory (or subdirectory) of one of the explicitly specified source
    files or in the directory (or subdirectory) of a remapping target.
    If you want to allow direct absolute includes, add the remapping ``/=/``.

.. index:: Remix IDE, file://

Using URLs in imports
~~~~~~~~~~~~~~~~~~~~~

Most URL prefixes such as ``https://`` or ``data://`` have no special meaning in import paths.
The only exception is ``file://`` which is stripped from source unit names by the default file
loader.

This does not mean you cannot use URLs as import paths at all.
While the command-line compiler will interpret an URL as a relative path (which will most likely fail),
the `JavaScript interface <https://github.com/ethereum/solc-js>`_ allows you to provide a callback
and implement your own, custom lookup rules, which may include supporting arbitrary URLs.
`The Remix IDE <https://remix.ethereum.org/>`_ uses this mechanism to allow files to be imported
directly from github:

.. code-block:: solidity
    :caption: contract.sol

    import "https://github.com/ethereum/dapp-bin/library/iterable_mapping.sol" as it_mapping;

When compiling locally you can use import remapping to replace the protocol and domain part with a
local path:

.. code-block:: bash

    solc :https://github.com/ethereum/dapp-bin=/usr/local/dapp-bin contract.sol

Note the leading ``:``.
It is necessary when the remapping context is empty.
Otherwise the ``https:`` part would be interpreted by the compiler as the context.

.. note::

    When remapping, keep in mind that the prefix must match exactly.
    ``https://example.com/project=/project`` will match  ``https://example.com/project/contract.sol``
    but not ``example.com/project/contract.sol``, ``https://example.com/project///contract.sol`` or
    ``https://EXAMPLE.COM/project/contract.sol``.

    Also, since URLs look to the compiler just like imports relative to base there is no
    normalization involved.
    The source unit ID for ``EXAMPLE.COM/project///contract.sol`` is exactly
    ``EXAMPLE.COM/project///contract.sol`` and not ``https://example.com/project/contract.sol``.
    It will only get normalized if the compiler passes the ID to the file loader but then the
    normalization rules for paths, not URLs will be applied.

.. note::

    ``file://`` prefix is stripped from import paths and from filesystem paths specified in
    ``sources.urls`` in Standard JSON. It is **not** stripped from filesystem paths provided on
    the command line.
    For example the following will not result in ``contract.sol`` being loaded:

    .. code-block:: bash

        solc file://contract.sol

    The compiler will instead try to find it in a directory called ``file:`` and fail if such a
    directory does not exist or does not contain ``contract.sol``.

.. index:: standard input, stdin, <stdin>

Standard Input
~~~~~~~~~~~~~~

The content of the standard input stream of the command-line compiler for all intents and purposes
behaves like a source file with an source unit ID of ``<stdin>`` placed directly in compiler's
virtual filesystem.
This means that:

- It can be imported like any other file from the virtual filesystem:

  .. code-block:: solidity

      import "<stdin>";

  .. note::

      If the compiler is not instructed to read content from its standard input by specyfing ``-``
      as one of the arguments, it will actually try to find a file called ``<stdin>`` in the
      filesystem when it encounters such an import.

- Paths in imports relative to source resolve into source unit IDs relative to base because
  ``<stdin>`` is not an absolute path.

  .. code-block:: solidity
      :caption: <stdin>

      import "./contract.sol"; // source unit ID: contract.sol
      import "../token.sol";   // source unit ID: token.sol

- It can be freely used in remappings. For example ``/project/contract.sol=<stdin>`` and
  ``<stdin>=contract.sol`` are both valid.


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
  (LF, VF, FF, CR, NEL, LS or PS) in UTF-8 encoding. The terminator is still part of
  the source code after the comment, so if it is not an ASCII symbol
  (these are NEL, LS and PS), it will lead to a parser error.

Additionally, there is another type of comment called a NatSpec comment,
which is detailed in the :ref:`style guide<style_guide_natspec>`. They are written with a
triple slash (``///``) or a double asterisk block (``/** ... */``) and
they should be used directly above function declarations or statements.
