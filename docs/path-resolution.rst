.. _path-resolution:

***************
Path Resolution
***************

In order to be able to support reproducible builds on all platforms, the Solidity compiler has to
abstract away the details of the filesystem where source files are stored.
Paths used in imports must work the same way everywhere while the command-line interface must be
able to work with platform-specific paths to provide good user experience.
This section aims to explain in detail how Solidity reconciles these requirements.

.. index:: ! virtual filesystem, ! VFS, ! source unit name
.. _virtual-filesystem:

Virtual Filesystem
==================

The compiler maintains an internal database (*virtual filesystem* or *VFS* for short) where each
source unit is assigned a unique *source unit name* which is an opaque and unstructured identifier.
When you use the :ref:`import statement <import>`, you specify an *import path* that references a
source unit name.

.. index:: ! import callback, ! Host Filesystem Loader
.. _import-callback:

Import Callback
---------------

The VFS is initially populated only with files the compiler has received as input.
Additional files can be loaded during compilation using an *import callback*.
If the compiler does not find any source unit name matching the import path in the VFS, it invokes
the callback, which is responsible for obtaining the source code to be placed under that name.
An import callback is free to interpret source unit names in an arbitrary way, not just a paths.
If there is no callback available when one is needed or if it fails to locate the source code,
compilation fails.

The command-line compiler provides the *Host Filesystem Loader* - a rudimentary callback
that interprets a source unit name as a path in the local filesystem.
The `JavaScript interface <https://github.com/ethereum/solc-js>`_ does not provide any by default,
but one can be provided by the user.
This mechanism can be used to obtain source code from locations other then the local filesystem
(which may not even be accessible, e.g. when the compiler is running in a browser).
For example the `Remix IDE <https://remix.ethereum.org/>`_ provides a versatile callback that
lets you `import files from HTTP, IPFS and Swarm URLs or refer directly to packages in NPM registry
<https://remix-ide.readthedocs.io/en/latest/import.html>`_.

.. note::

    Host Filesystem Loader's file lookup is platform-dependent.
    For example backslashes in source unit name can be interpreted as directory separators or not
    and the lookup can be case-sensitive or not, depending on the underlying platform.

    For portability it is recommended to avoid using import paths that will work correctly only
    with a specific import callback or only on one platform.

.. index:: ! CLI path

Populating the Virtual Filesystem
---------------------------------

The initial content of the VFS depends on how you invoke the compiler:

#. **CLI**

   When you compile a file using the command-line interface of the compiler, you provide one or
   more *CLI paths* to files containing Solidity code:

   .. code-block:: bash

       solc contract.sol /usr/local/dapp-bin/token.sol

   The source unit name of a file loaded this way is simply the specified CLI path with
   platform-specific separators converted to forward slashes.
   There is no normalization beyond that.
   Multiple slashes and ``./`` and ``../`` segments all remain intact.
   Relative paths are also **not** converted into absolute ones so ``solc /project/contract.sol``
   and ``solc contract.sol`` will result in two different source unit names even if you run the
   compiler from within ``/project``.

   .. note::

       Because CLI paths are platform-specific, The same path may be interpreted differently on
       different systems.

       .. code-block:: shell
           :caption: Windows

           solc.exe C:\project\token.sol &REM source unit name: C:/project/token.sol
           solc.exe /project/token.sol   &REM source unit name: /project/token.sol

       .. code-block:: bash
           :caption: Linux

           solc C:\project\token.sol # source unit name: C:projecttoken.sol
           solc /project/token.sol   # source unit name: /project/token.sol

       For this reason it is recommended to use forward slashes on all platforms and refrain from
       using absolute paths.

   .. index:: standard JSON

#. **Standard JSON**

   When using the :ref:`Standard JSON <compiler-api>` API (via either the `JavaScript interface
   <https://github.com/ethereum/solc-js>`_ or the ``--standard-json`` command-line option)
   you provide input in JSON format, containing, among other things, the content of all your source
   files:

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

   The ``sources`` dictionary becomes the initial content of the virtual filesystem and its keys
   are used as source unit names.

   With ``--standard-json`` the path to the JSON file does not affect the path resolution in any way.
   In fact, it is common to supply it on the standard input in which case it does not have a path at all.

#. **Standard JSON (via import callback)**

   With Standard JSON it is also possible to tell the compiler to use the import callback to obtain
   the source code:

   .. code-block:: json

       {
           "language": "Solidity",
           "sources": {
               "/usr/local/dapp-bin/token.sol": {
                   "urls": [
                       "/projects/mytoken.sol",
                       "https://example.com/projects/mytoken.sol"
                   ]
               }
           },
           "settings": {"outputSelection": {"*": { "*": ["metadata", "evm.bytecode"]}}}
       }

   If an import callback is available, the compiler will pass it the source unit names specified in
   ``urls`` one by one, until one is loaded successfully or the end of the list is reached.

   The source unit names are determined the same way as when using ``content`` - they are keys of
   the ``sources`` dictionary and the content of ``urls`` does not affect them in any way.

   .. note::

       When the Host Filesystem Loader is the callback, paths in ``urls`` are affected by
       :ref:`base path <base-path>`, and any other transformations performed by it.

   .. index:: standard input, stdin, <stdin>

#. **Standard input**

   On the command line it is also possible to provide the source is by sending it to compiler's
   :ref:`standard input <standard-input>`:

   .. code-block:: bash

       echo 'import "./util.sol"; contract C {}' | solc -

   The content of the standard input is placed in the virtual filesystem under a special source
   unit name: ``<stdin>``.

Once the VFS is initialized, additional files can still be added to it only through the import
callback.

.. index:: ! import; path

Imports
=======

The import statement specifies an *import path*, which, after being lightly processed, becomes a
source unit name.
Based on how the processing is performed, we can divide imports into two categories:

- :ref:`Direct imports <direct-imports>`, where you specify the full source unit name.
- :ref:`Relative imports <relative-imports>`, where you specify a path to be combined with the
  source unit name of the importing file.

.. warning::

    The compiler uses source unit names to determine whether imports refer to the same source unit or not.
    If you refer to a file in multiple ways that translate to different names, it will be compiled
    multiple times.

    For example:

    .. code-block:: solidity
        :caption: /code/contract.sol

        import "tokens/token.sol" as token1;   // source unit name: tokens/token.sol
        import "tokens///token.sol" as token2; // source unit name: tokens///token.sol

    .. code-block:: bash

        cd /code
        solc contract.sol /code/tokens/token.sol # source unit name: /code/tokens/token.sol

    In the above ``token.sol`` will end up in the virtual filesystem under three different
    source unit names even though all the paths refer to the same file in the underlying filesystem.

    To avoid this situation it is recommended to always use the canonical form of paths in your
    imports and to only list the top-level files that are not imported by other files when
    invoking the CLI compiler.

.. index:: ! direct import, import; direct
.. _direct-imports:

Direct Imports
--------------

An import that does not start with ``./`` or ``../`` is a *direct import*.

::

    import "/project/lib/util.sol";         // source unit name: /project/lib/util.sol
    import "lib/util.sol";                  // source unit name: lib/util.sol
    import "@openzeppelin/address.sol";     // source unit name: @openzeppelin/address.sol
    import "https://example.com/token.sol"; // source unit name: https://example.com/token.sol

The import path translates directly to a source unit name without normalization of any kind:

::

    import "/project/lib/../lib///math.sol"; // source unit name: /project/lib/../lib///math.sol
    import "lib/../lib///math.sol";          // source unit name: lib/../lib///math.sol

In the above you might expect the source unit names to be ``/project/lib/math.sol`` and
``lib/math.sol`` respectively but this is not the case.
For direct imports the source unit name is exactly what is stated in the import (unless
:ref:`remappings <import-remapping>` are used).
When the source is provided via Standard JSON interface each of these names can actually be
associated with different content.

When the source is not available in the virtual filesystem, the compiler passes the source unit name
to the import callback.
The Host Filesystem Loader will attempt to use it as a path and look up the file on disk.
At this point the platform-specific normalization rules kick in and ``/project/lib/math.sol`` and
``/project/lib/../lib///math.sol`` may actually result in the same file being loaded.
Note, however, that the compiler will still see them as separate source units that just happen to
have identical content.

.. note::

    While the rules for translating import paths into source unit names are the same on every
    platform, the Host Filesystem Loader uses platform-specific rules to locate files on disk.
    This means that for example this import might result in the file being successfully loaded from
    disk when compiling on Windows but not on other platforms:

    .. code-block:: solidity

        import "C:\\project\\lib\\token.sol"; // source unit name: C:\project\lib\token.sol

    To compile such a project on a different platform you would have to use the Standard JSON
    interface and provide the source directly under the right source unit name.
    For this reason relying on platform-specific behaviour of an import callback is highly discouraged.

.. index:: ! relative import, ! import; relative
.. _relative-imports:

Relative Imports
----------------

An import starting with ``./`` or ``../`` is a *relative import*.
Such imports specify a path relative to the source unit name of the importing source unit:

.. code-block:: solidity
    :caption: /project/lib/math.sol

    import "./util.sol" as util;    // source unit name: /project/lib/util.sol
    import "../token.sol" as token; // source unit name: /project/token.sol

.. code-block:: solidity
    :caption: lib/math.sol

    import "./util.sol" as util;    // source unit name: lib/util.sol
    import "../token.sol" as token; // source unit name: token.sol

.. note::

    Do not confuse relative imports with relative paths.
    Both ``util.sol`` and ``./util.sol`` specify relative paths on disk but these paths are treated
    very differently when used in imports.
    Only the latter creates a relative import.

    Consider the following example:

    .. code-block:: solidity
       :caption: /project/lib/math.sol

       import "/project/lib/util.sol" as util1; // source unit name: /project/lib/util.sol
       import "./util.sol" as util2;            // source unit name: /project/lib/util.sol
       import "util.sol" as util3;              // source unit name: util.sol

    In the situation above the first and the second import are equivalent and refer to the same
    source unit in the virtual filesystem.
    The compiler will recognize that the source has already been loaded when it encounters
    ``./util.sol`` and will not try to load it again.
    This is not the case with the third import.
    When asked for ``util.sol`` with a direct import, the compiler will try to find exactly that.
    The entry with the source unit name of ``/project/lib/util.sol`` will not be used.

    Even if you run the compiler from within ``/project/lib/`` the relative ``util.sol`` will only
    get resolved into ``/project/lib/util.sol`` by the Host Filesystem Loader.
    When the callback returns the source, the compiler will still place it under ``util.sol`` and not
    ``/project/lib/util.sol`` in the virtual filesystem.

Unlike in direct imports, the paths used in relative imports do get normalized.
The normalization rules are the same as for UNIX paths, namely:

- All the ``./`` segments are removed.
- Every ``../``  segment backtracks one level up in the hierarchy.
- Multiple slashes are squashed into a single one.

Example:

.. code-block:: solidity
    :caption: lib/contract.sol

    import "./util/./util.sol";         // source unit name: lib/util/util.sol
    import "./util//util.sol";          // source unit name: lib/util/util.sol
    import "../util/../array/util.sol"; // source unit name: array/util.sol

.. warning::

    The root of the virtual filesystem is an empty path, not ``/``.
    This matters when the ``../`` segments go beyond the root.
    In UNIX paths such segments are ignored and for example ``/../../`` is
    equivalent to just ``/``.
    In the virtual filesystem the rule is similar but the result is an empty path instead.

    .. code-block:: solidity
        :caption: /project/lib/contract.sol

        import "../util.sol";          // source unit name: /project/util.sol
        import "../../util.sol";       // source unit name: /util.sol
        import "../../../util.sol";    // source unit name: util.sol
        import "../../../../util.sol"; // source unit name: util.sol

.. note::

    The importing source unit name is **not** normalized.
    This ensures that relative imports work properly when the importing file is identified with a URL:

    .. code-block:: solidity
        :caption: https://example.com/contract.sol

        import "./token.sol"; // source unit name: https://example.com/token.sol

    If the importing source unit name were to be normalized, the name would become
    ``https:/example.com/token.sol`` which is not a valid URL.

.. warning::

    The ``./`` and ``../`` segments in the importing source unit name have no special meaning.

    .. code-block:: solidity
        :caption: ../lib/../lib/math.sol

        import "./util.sol" as util;    // source unit name: ../lib/../lib/util.sol
        import "../token.sol" as token; // source unit name: ../lib/../../token.sol

    This may lead to surprising results in corner cases.
    For example they can get canceled by ``../`` segments in the import path:

    .. code-block:: solidity
        :caption: /project/./lib/contract.sol

        import "../util.sol";       // source unit name: /project/./util.sol
        import "../../util.sol";    // source unit name: /project/util.sol
        import "../../../util.sol"; // source unit name: /util.sol

.. note::

    The use of relative imports containing leading ``../`` segments is not recommended.
    The same effect can be achieved in a more reliable way by using direct imports with
    :ref:`base path <base-path>` and :ref:`import remapping <import-remapping>`.

.. index:: ! base path, --base-path
.. _base-path:

Base Path
=========

Base path specifies the directory that the Host Filesystem Loader will load files from.
It is simply prepended to a source unit name before the filesystem lookup is performed.

By default base path is empty, which results in the files being looked up in the directory the
compiler has been invoked from when the source unit name is a relative path or in arbitrary
places in the filesystem when it is an absolute one:

.. code-block:: solidity
    :caption: lib/parent.sol

    import "./util.sol";        // source unit name: lib/util.sol
    import "token.sol";         // source unit name: token.sol
    import "/tmp/contract.sol"; // source unit name: /tmp/contract.sol

.. code-block:: bash

    cd /home/user
    solc lib/parent.sol # source unit name: lib/parent.sol

In the example above the compiler will attempt to load the following files:

+-------------------------+-----------------------------------------------------------------+
| Source unit name        | Filesystem path                                                 |
+=========================+=================================================================+
| ``lib/parent.sol``      + ``/home/user/lib/parent.sol``                                   |
+-------------------------+-----------------------------------------------------------------+
| ``lib/util.sol``        + ``/home/user/lib/util.sol``                                     |
+-------------------------+-----------------------------------------------------------------+
| ``token.sol``           + ``/home/user/token.sol``                                        |
+-------------------------+-----------------------------------------------------------------+
| ``/tmp/contract.sol``   + ``/tmp/contract.sol``                                           |
+-------------------------+-----------------------------------------------------------------+

If you want to run the compiler from a different directory, you can use ``--base-path`` option to
explicitly set the location of the project root:

.. code-block:: bash

    solc /project/contract.sol --base-path /project # source unit name: lib/parent.sol

+-------------------------+-----------------------------------------------------------------+
| Source unit name        | Filesystem path                                                 |
+=========================+=================================================================+
| ``lib/parent.sol``      + ``/home/user/lib/parent.sol``                                   |
+-------------------------+-----------------------------------------------------------------+
| ``lib/util.sol``        + ``/project/lib/util.sol``                                       |
+-------------------------+-----------------------------------------------------------------+
| ``token.sol``           + ``/project/token.sol``                                          |
+-------------------------+-----------------------------------------------------------------+
| ``/tmp/contract.sol``   + ``/project/tmp/contract.sol``                                   |
+-------------------------+-----------------------------------------------------------------+

.. note::

    Base path does not affect paths you specify directly on the command line.
    It is a feature of the Host Filesystem Loader so it is prepended only to the source unit names
    that are passed to this specific import callback, i.e. the ones that come from imports and
    ``source.urls`` in Standard JSON.

.. note::

    Base path is prepended no matter whether an import contains a relative or an absolute path.
    This may not be apparent because the default value of the option is an empty path.

.. note::

    If you set base path to a relative path, it is interpreted as relative to the current working directory.
    Note that if you do this, all absolute paths will effectively be converted into relative ones
    by the Host Filesystem Loader.
    For example ``import "/project/contract.sol"`` with base path set to ``lib/token`` will result
    in the loader looking for ``lib/token/project/contract.sol`` in the current working directory.

.. index:: ! remapping; import, ! import; remapping, ! remapping; context, ! remapping; prefix, ! remapping; target
.. _import-remapping:

Import Remapping
================

Base path and relative imports on their own allow you to freely move your project around the
filesystem but force you to keep all files within a single directory and its subdirectories.
When using external libraries it is often desirable to keep their files in a separate location.
To help with that, the compiler provides another mechanism: import remapping.

Remapping allows you to have the compiler replace import path prefixes with something else.
For example you can set up a remapping so that everything imported from the virtual directory
``github.com/ethereum/dapp-bin/library`` would actually receive source unit names starting with
``dapp-bin/library``.
By setting base path to ``/project`` you could then have the compiler find them in
``/project/dapp-bin/library``

The remappings can depend on a context, which allows you to configure packages to import,
e.g. different versions of a library of the same name.

.. warning::

    Information about used remappings is stored in contract metadata so modifying them will result
    in a slightly different bytecode.
    This means that if you move your project files to different locations and use remappings to
    avoid having to adjust the source code, your project will compile but will no longer produce the
    exact same bytecode as without the remappings.

Import remappings have the form of ``context:prefix=target``.
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

#. **Remappings only affect the translation between import paths and source unit names.**

   Source unit names added via other means cannot be remapped.
   For example the paths you specify on the command-line and the ones in ``sources.urls`` in
   Standard JSON are not affected.

    .. code-block:: bash

        solc /project=/contracts /project/contract.sol # source unit name: /project/contract.sol

#. **Context and prefix must match source unit names, not import paths.**

   - This means that you cannot remap ``./`` or ``../`` directly since they are replaced during
     the translation to source unit name but you can remap the part of the name they are replaced
     with:

     .. code-block:: bash

         solc ./=a /project=b /project/contract.sol

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "./util.sol" as util; // source unit name: b/util.sol

   - You cannot remap base path or any other part of the path that is only added internally by an
     import callback:

     .. code-block:: bash

         solc /project=/contracts /project/contract.sol --base-path /project

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "util.sol" as util; // source unit name: util.sol

#. **Target is inserted directly into the source unit name and does not necessarily have to be a valid path.**

   - It can be anything as long as the import callback can handle it.
     In case of the Host Filesystem Loader this includes also relative paths.
     When using the JavaScript interface you can even use URLs and abstract identifiers if
     your callback can handle them.

   - Remapping happens after relative imports have already been resolved into source unit names.
     This means that targets starting with ``./`` and ``../`` have no special meaning and are
     relative to the base path rather than to the location of the source file.

   - Remapping targets are not normalized so ``@root=./a/b//`` will remap ``@root/contract.sol``
     to ``./a/b//contract.sol`` and not ``a/b/contract.sol``.

   - If the target does not end with a slash, the compiler will not add one automatically:

     .. code-block:: bash

         solc /project/=/contracts /project/contract.sol

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "/project/util.sol" as util; // source unit name: /contractsutil.sol

#. **Context and prefix are patterns and matches must be exact.**

   - ``a//b=c`` will not match ``a/b``.

   - source unit names are not normalized so ``a/b=c`` will not match ``a//b`` either.

   - Parts of file and directory names can match as well.
     ``/newProject/con:/new=old`` will match ``/newProject/contract.sol`` and remap it to
     ``oldProject/contract.sol``.

#. **At most one remapping is applied to a single import.**

   - If multiple remappings match the same source unit name, the one with the longest matching
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
=====================

Most URL prefixes such as ``https://`` or ``data://`` have no special meaning in import paths.
The only exception is ``file://`` which is stripped from source unit names by the Host Filesystem
Loader.

This does not mean you cannot use URLs as import paths at all.
While the Host Filesystem Loader will interpret a URL as a relative path (which will most likely fail),
the `JavaScript interface <https://github.com/ethereum/solc-js>`_ allows you to provide a custom
import callback and implement your own lookup rules, which may include supporting arbitrary URLs.
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

    Also, since using a URL as the import path results in a direct import, there is no
    normalization involved.
    The source unit name for ``EXAMPLE.COM/project///contract.sol`` is exactly
    ``EXAMPLE.COM/project///contract.sol`` and not ``https://example.com/project/contract.sol``.
    It would get normalized by the Host Filesystem Loader but only according to normalization rules for
    paths, and would not be actually interpreted as a URL in that situation.

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
.. _standard-input:

Standard Input
==============

The content of the standard input stream of the command-line compiler for all intents and purposes
behaves like a source file with an source unit name of ``<stdin>`` placed directly in compiler's
virtual filesystem.
This means that:

- It can be imported like any other file from the virtual filesystem:

  .. code-block:: solidity

      import "<stdin>";

  .. note::

      If the compiler is not instructed to read content from its standard input by specyfing ``-``
      as one of the arguments, it will actually try to find a file called ``<stdin>`` in the
      filesystem when it encounters such an import.

- Paths in relative imports resolve into relative source unit names because the importing source unit
  name (``<stdin>``) is not an absolute path:

  .. code-block:: solidity
      :caption: <stdin>

      import "./contract.sol"; // source unit name: contract.sol
      import "../token.sol";   // source unit name: token.sol

- It can be freely used in remappings. For example ``/project/contract.sol=<stdin>`` and
  ``<stdin>=contract.sol`` are both valid.
