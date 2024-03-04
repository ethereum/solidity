.. _path-resolution:

**********************
Import Path Resolution
**********************

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

.. index:: ! import callback, ! Host Filesystem Loader, ! --no-import-callback
.. _import-callback:

Import Callback
---------------

The VFS is initially populated only with files the compiler has received as input.
Additional files can be loaded during compilation using an *import callback*, which is different
depending on the type of compiler you use (see below).
If the compiler does not find any source unit name matching the import path in the VFS, it invokes
the callback, which is responsible for obtaining the source code to be placed under that name.
An import callback is free to interpret source unit names in an arbitrary way, not just as paths.
If there is no callback available when one is needed or if it fails to locate the source code,
compilation fails.

By default, the command-line compiler provides the *Host Filesystem Loader* - a rudimentary callback
that interprets a source unit name as a path in the local filesystem.
This callback can be disabled using the ``--no-import-callback`` command-line option.
The `JavaScript interface <https://github.com/ethereum/solc-js>`_ does not provide any by default,
but one can be provided by the user.
This mechanism can be used to obtain source code from locations other than the local filesystem
(which may not even be accessible, e.g. when the compiler is running in a browser).
For example the `Remix IDE <https://remix.ethereum.org/>`_ provides a versatile callback that
lets you `import files from HTTP, IPFS and Swarm URLs or refer directly to packages in NPM registry
<https://remix-ide.readthedocs.io/en/latest/import.html>`_.

.. note::

    Host Filesystem Loader's file lookup is platform-dependent.
    For example backslashes in a source unit name can be interpreted as directory separators or not
    and the lookup can be case-sensitive or not, depending on the underlying platform.

    For portability it is recommended to avoid using import paths that will work correctly only
    with a specific import callback or only on one platform.
    For example you should always use forward slashes since they work as path separators also on
    platforms that support backslashes.

Initial Content of the Virtual Filesystem
-----------------------------------------

The initial content of the VFS depends on how you invoke the compiler:

#. **solc / command-line interface**

   When you compile a file using the command-line interface of the compiler, you provide one or
   more paths to files containing Solidity code:

   .. code-block:: bash

       solc contract.sol /usr/local/dapp-bin/token.sol

   The source unit name of a file loaded this way is constructed by converting its path to a
   canonical form and, if possible, making it relative to either the base path or one of the
   include paths.
   See :ref:`CLI Path Normalization and Stripping <cli-path-normalization-and-stripping>` for
   a detailed description of this process.

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

   .. _initial-vfs-content-standard-json-with-import-callback:

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

   If an import callback is available, the compiler will give it the strings specified in
   ``urls`` one by one, until one is loaded successfully or the end of the list is reached.

   The source unit names are determined the same way as when using ``content`` - they are keys of
   the ``sources`` dictionary and the content of ``urls`` does not affect them in any way.

   .. index:: standard input, stdin, <stdin>

#. **Standard input**

   On the command-line it is also possible to provide the source by sending it to compiler's
   standard input:

   .. code-block:: bash

       echo 'import "./util.sol"; contract C {}' | solc -

   ``-`` used as one of the arguments instructs the compiler to place the content of the standard
   input in the virtual filesystem under a special source unit name: ``<stdin>``.

Once the VFS is initialized, additional files can still be added to it only through the import
callback.

.. index:: ! import; path

Imports
=======

The import statement specifies an *import path*.
Based on how the import path is specified, we can divide imports into two categories:

- :ref:`Direct imports <direct-imports>`, where you specify the full source unit name directly.
- :ref:`Relative imports <relative-imports>`, where you specify a path starting with ``./`` or ``../``
  to be combined with the source unit name of the importing file.


.. code-block:: solidity
    :caption: contracts/contract.sol

    import "./math/math.sol";
    import "contracts/tokens/token.sol";

In the above ``./math/math.sol`` and ``contracts/tokens/token.sol`` are import paths while the
source unit names they translate to are ``contracts/math/math.sol`` and ``contracts/tokens/token.sol``
respectively.

.. index:: ! direct import, import; direct
.. _direct-imports:

Direct Imports
--------------

An import that does not start with ``./`` or ``../`` is a *direct import*.

.. code-block:: solidity

    import "/project/lib/util.sol";         // source unit name: /project/lib/util.sol
    import "lib/util.sol";                  // source unit name: lib/util.sol
    import "@openzeppelin/address.sol";     // source unit name: @openzeppelin/address.sol
    import "https://example.com/token.sol"; // source unit name: https://example.com/token.sol

After applying any :ref:`import remappings <import-remapping>` the import path simply becomes the
source unit name.

.. note::

    A source unit name is just an identifier and even if its value happens to look like a path, it
    is not subject to the normalization rules you would typically expect in a shell.
    Any ``/./`` or ``/../`` segments or sequences of multiple slashes remain a part of it.
    When the source is provided via Standard JSON interface it is entirely possible to associate
    different content with source unit names that would refer to the same file on disk.

When the source is not available in the virtual filesystem, the compiler passes the source unit name
to the import callback.
The Host Filesystem Loader will attempt to use it as a path and look up the file on disk.
At this point the platform-specific normalization rules kick in and names that were considered
different in the VFS may actually result in the same file being loaded.
For example ``/project/lib/math.sol`` and ``/project/lib/../lib///math.sol`` are considered
completely different in the VFS even though they refer to the same file on disk.

.. note::

    Even if an import callback ends up loading source code for two different source unit names from
    the same file on disk, the compiler will still see them as separate source units.
    It is the source unit name that matters, not the physical location of the code.

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

    Relative imports **always** start with ``./`` or ``../`` so ``import "util.sol"``, unlike
    ``import "./util.sol"``, is a direct import.
    While both paths would be considered relative in the host filesystem, ``util.sol`` is actually
    absolute in the VFS.

Let us define a *path segment* as any non-empty part of the path that does not contain a separator
and is bounded by two path separators.
A separator is a forward slash or the beginning/end of the string.
For example in ``./abc/..//`` there are three path segments: ``.``, ``abc`` and ``..``.

The compiler resolves the import into a source unit name based on the import path, in the following way:

#. We start with the source unit name of the importing source unit.
#. The last path segment with preceding slashes is removed from the resolved name.
#. Then, for every segment in the import path, starting from the leftmost one:

    - If the segment is ``.``, it is skipped.
    - If the segment is ``..``, the last path segment with preceding slashes is removed from the resolved name.
    - Otherwise, the segment (preceded by a single slash if the resolved name is not empty), is appended to the resolved name.

The removal of the last path segment with preceding slashes is understood to
work as follows:

1. Everything past the last slash is removed (i.e. ``a/b//c.sol`` becomes ``a/b//``).
2. All trailing slashes are removed (i.e. ``a/b//`` becomes ``a/b``).

Note that the process normalizes the part of the resolved source unit name that comes from the import path according
to the usual rules for UNIX paths, i.e. all ``.`` and ``..`` are removed and multiple slashes are
squashed into a single one.
On the other hand, the part that comes from the source unit name of the importing module remains unnormalized.
This ensures that the ``protocol://`` part does not turn into ``protocol:/`` if the importing file
is identified with a URL.

If your import paths are already normalized, you can expect the above algorithm to produce very
intuitive results.
Here are some examples of what you can expect if they are not:

.. code-block:: solidity
    :caption: lib/src/../contract.sol

    import "./util/./util.sol";         // source unit name: lib/src/../util/util.sol
    import "./util//util.sol";          // source unit name: lib/src/../util/util.sol
    import "../util/../array/util.sol"; // source unit name: lib/src/array/util.sol
    import "../.././../util.sol";       // source unit name: util.sol
    import "../../.././../util.sol";    // source unit name: util.sol

.. note::

    The use of relative imports containing leading ``..`` segments is not recommended.
    The same effect can be achieved in a more reliable way by using direct imports with
    :ref:`base path and include paths <base-and-include-paths>`.

.. index:: ! base path, ! --base-path, ! include paths, ! --include-path
.. _base-and-include-paths:

Base Path and Include Paths
===========================

The base path and include paths represent directories that the Host Filesystem Loader will load files from.
When a source unit name is passed to the loader, it prepends the base path to it and performs a
filesystem lookup.
If the lookup does not succeed, the same is done with all directories on the include path list.

It is recommended to set the base path to the root directory of your project and use include paths to
specify additional locations that may contain libraries your project depends on.
This lets you import from these libraries in a uniform way, no matter where they are located in the
filesystem relative to your project.
For example, if you use npm to install packages and your contract imports
``@openzeppelin/contracts/utils/Strings.sol``, you can use these options to tell the compiler that
the library can be found in one of the npm package directories:

.. code-block:: bash

    solc contract.sol \
        --base-path . \
        --include-path node_modules/ \
        --include-path /usr/local/lib/node_modules/

Your contract will compile (with the same exact metadata) no matter whether you install the library
in the local or global package directory or even directly under your project root.

By default the base path is empty, which leaves the source unit name unchanged.
When the source unit name is a relative path, this results in the file being looked up in the
directory the compiler has been invoked from.
It is also the only value that results in absolute paths in source unit names being actually
interpreted as absolute paths on disk.
If the base path itself is relative, it is interpreted as relative to the current working directory
of the compiler.

.. note::

    Include paths cannot have empty values and must be used together with a non-empty base path.

.. note::

    Include paths and base path can overlap as long as it does not make import resolution ambiguous.
    For example, you can specify a directory inside base path as an include directory or have an
    include directory that is a subdirectory of another include directory.
    The compiler will only issue an error if the source unit name passed to the Host Filesystem
    Loader represents an existing path when combined with multiple include paths or an include path
    and base path.

.. _cli-path-normalization-and-stripping:

CLI Path Normalization and Stripping
------------------------------------

On the command-line the compiler behaves just as you would expect from any other program:
it accepts paths in a format native to the platform and relative paths are relative to the current
working directory.
The source unit names assigned to files whose paths are specified on the command-line, however,
should not change just because the project is being compiled on a different platform or because the
compiler happens to have been invoked from a different directory.
To achieve this, paths to source files coming from the command-line must be converted to a canonical
form, and, if possible, made relative to the base path or one of the include paths.

The normalization rules are as follows:

- If a path is relative, it is made absolute by prepending the current working directory to it.
- Internal ``.`` and ``..`` segments are collapsed.
- Platform-specific path separators are replaced with forward slashes.
- Sequences of multiple consecutive path separators are squashed into a single separator (unless
  they are the leading slashes of an `UNC path <https://en.wikipedia.org/wiki/Path_(computing)#UNC>`_).
- If the path includes a root name (e.g. a drive letter on Windows) and the root is the same as the
  root of the current working directory, the root is replaced with ``/``.
- Symbolic links in the path are **not** resolved.

  - The only exception is the path to the current working directory prepended to relative paths in
    the process of making them absolute.
    On some platforms the working directory is reported always with symbolic links resolved so for
    consistency the compiler resolves them everywhere.

- The original case of the path is preserved even if the filesystem is case-insensitive but
  `case-preserving <https://en.wikipedia.org/wiki/Case_preservation>`_ and the actual case on
  disk is different.

.. note::

    There are situations where paths cannot be made platform-independent.
    For example on Windows the compiler can avoid using drive letters by referring to the root
    directory of the current drive as ``/`` but drive letters are still necessary for paths leading
    to other drives.
    You can avoid such situations by ensuring that all the files are available within a single
    directory tree on the same drive.

After normalization the compiler attempts to make the source file path relative.
It tries the base path first and then the include paths in the order they were given.
If the base path is empty or not specified, it is treated as if it was equal to the path to the
current working directory (with all symbolic links resolved).
The result is accepted only if the normalized directory path is the exact prefix of the normalized
file path.
Otherwise the file path remains absolute.
This makes the conversion unambiguous and ensures that the relative path does not start with ``../``.
The resulting file path becomes the source unit name.

.. note::

    The relative path produced by stripping must remain unique within the base path and include paths.
    For example the compiler will issue an error for the following command if both
    ``/project/contract.sol`` and ``/lib/contract.sol`` exist:

    .. code-block:: bash

        solc /project/contract.sol --base-path /project --include-path /lib

.. note::

    Prior to version 0.8.8, CLI path stripping was not performed and the only normalization applied
    was the conversion of path separators.
    When working with older versions of the compiler it is recommended to invoke the compiler from
    the base path and to only use relative paths on the command-line.

.. index:: ! allowed paths, ! --allow-paths, remapping; target
.. _allowed-paths:

Allowed Paths
=============

As a security measure, the Host Filesystem Loader will refuse to load files from outside of a few
locations that are considered safe by default:

- Outside of Standard JSON mode:

  - The directories containing input files listed on the command-line.
  - The directories used as :ref:`remapping <import-remapping>` targets.
    If the target is not a directory (i.e does not end with ``/``, ``/.`` or ``/..``) the directory
    containing the target is used instead.
  - Base path and include paths.

- In Standard JSON mode:

  - Base path and include paths.

Additional directories can be whitelisted using the ``--allow-paths`` option.
The option accepts a comma-separated list of paths:

.. code-block:: bash

    cd /home/user/project/
    solc token/contract.sol \
        lib/util.sol=libs/util.sol \
        --base-path=token/ \
        --include-path=/lib/ \
        --allow-paths=../utils/,/tmp/libraries

When the compiler is invoked with the command shown above, the Host Filesystem Loader will allow
importing files from the following directories:

- ``/home/user/project/token/`` (because ``token/`` contains the input file and also because it is
  the base path),
- ``/lib/`` (because ``/lib/`` is one of the include paths),
- ``/home/user/project/libs/`` (because ``libs/`` is a directory containing a remapping target),
- ``/home/user/utils/`` (because of ``../utils/`` passed to ``--allow-paths``),
- ``/tmp/libraries/`` (because of ``/tmp/libraries`` passed to ``--allow-paths``),

.. note::

    The working directory of the compiler is one of the paths allowed by default only if it
    happens to be the base path (or the base path is not specified or has an empty value).

.. note::

    The compiler does not check if allowed paths actually exist and whether they are directories.
    Non-existent or empty paths are simply ignored.
    If an allowed path matches a file rather than a directory, the file is considered whitelisted, too.

.. note::

    Allowed paths are case-sensitive even if the filesystem is not.
    The case must exactly match the one used in your imports.
    For example ``--allow-paths tokens`` will not match ``import "Tokens/IERC20.sol"``.

.. warning::

    Files and directories only reachable through symbolic links from allowed directories are not
    automatically whitelisted.
    For example if ``token/contract.sol`` in the example above was actually a symlink pointing at
    ``/etc/passwd`` the compiler would refuse to load it unless ``/etc/`` was one of the allowed
    paths too.

.. index:: ! remapping; import, ! import; remapping, ! remapping; context, ! remapping; prefix, ! remapping; target
.. _import-remapping:

Import Remapping
================

Import remapping allows you to redirect imports to a different location in the virtual filesystem.
The mechanism works by changing the translation between import paths and source unit names.
For example you can set up a remapping so that any import from the virtual directory
``github.com/ethereum/dapp-bin/library/`` would be seen as an import from ``dapp-bin/library/`` instead.

You can limit the scope of a remapping by specifying a *context*.
This allows creating remappings that apply only to imports located in a specific library or a specific file.
Without a context a remapping is applied to every matching import in all the files in the virtual
filesystem.

Import remappings have the form of ``context:prefix=target``:

- ``context`` must match the beginning of the source unit name of the file containing the import.
- ``prefix`` must match the beginning of the source unit name resulting from the import.
- ``target`` is the value the prefix is replaced with.

For example, if you clone https://github.com/ethereum/dapp-bin/ locally to ``/project/dapp-bin``
and run the compiler with:

.. code-block:: bash

    solc github.com/ethereum/dapp-bin/=dapp-bin/ --base-path /project source.sol

you can use the following in your source file:

.. code-block:: solidity

    import "github.com/ethereum/dapp-bin/library/math.sol"; // source unit name: dapp-bin/library/math.sol

The compiler will look for the file in the VFS under ``dapp-bin/library/math.sol``.
If the file is not available there, the source unit name will be passed to the Host Filesystem
Loader, which will then look in ``/project/dapp-bin/library/math.sol``.

.. warning::

    Information about remappings is stored in contract metadata.
    Since the binary produced by the compiler has a hash of the metadata embedded in it, any
    modification to the remappings will result in different bytecode.

    For this reason you should be careful not to include any local information in remapping targets.
    For example if your library is located in ``/home/user/packages/mymath/math.sol``, a remapping
    like ``@math/=/home/user/packages/mymath/`` would result in your home directory being included in
    the metadata.
    To be able to reproduce the same bytecode with such a remapping on a different machine, you
    would need to recreate parts of your local directory structure in the VFS and (if you rely on
    Host Filesystem Loader) also in the host filesystem.

    To avoid having your local directory structure embedded in the metadata, it is recommended to
    designate the directories containing libraries as *include paths* instead.
    For example, in the example above ``--include-path /home/user/packages/`` would let you use
    imports starting with ``mymath/``.
    Unlike remapping, the option on its own will not make ``mymath`` appear as ``@math`` but this
    can be achieved by creating a symbolic link or renaming the package subdirectory.

As a more complex example, suppose you rely on a module that uses an old version of dapp-bin that
you checked out to ``/project/dapp-bin_old``, then you can run:

.. code-block:: bash

    solc module1:github.com/ethereum/dapp-bin/=dapp-bin/ \
         module2:github.com/ethereum/dapp-bin/=dapp-bin_old/ \
         --base-path /project \
         source.sol

This means that all imports in ``module2`` point to the old version but imports in ``module1``
point to the new version.

Here are the detailed rules governing the behavior of remappings:

#. **Remappings only affect the translation between import paths and source unit names.**

   Source unit names added to the VFS in any other way cannot be remapped.
   For example the paths you specify on the command-line and the ones in ``sources.urls`` in
   Standard JSON are not affected.

   .. code-block:: bash

       solc /project/=/contracts/ /project/contract.sol # source unit name: /project/contract.sol

   In the example above the compiler will load the source code from ``/project/contract.sol`` and
   place it under that exact source unit name in the VFS, not under ``/contract/contract.sol``.

#. **Context and prefix must match source unit names, not import paths.**

   - This means that you cannot remap ``./`` or ``../`` directly since they are replaced during
     the translation to source unit name but you can remap the part of the name they are replaced
     with:

     .. code-block:: bash

         solc ./=a/ /project/=b/ /project/contract.sol # source unit name: /project/contract.sol

     .. code-block:: solidity
         :caption: /project/contract.sol

         import "./util.sol" as util; // source unit name: b/util.sol

   - You cannot remap base path or any other part of the path that is only added internally by an
     import callback:

     .. code-block:: bash

         solc /project/=/contracts/ /project/contract.sol --base-path /project # source unit name: contract.sol

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

   - Remapping targets are not normalized so ``@root/=./a/b//`` will remap ``@root/contract.sol``
     to ``./a/b//contract.sol`` and not ``a/b/contract.sol``.

   - If the target does not end with a slash, the compiler will not add one automatically:

     .. code-block:: bash

         solc /project/=/contracts /project/contract.sol # source unit name: /project/contract.sol

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

   - If ``target`` is the empty string, ``prefix`` is simply removed from import paths.
   - Empty ``context`` means that the remapping applies to all imports in all source units.

.. index:: Remix IDE, file://

Using URLs in imports
=====================

Most URL prefixes such as ``https://`` or ``data://`` have no special meaning in import paths.
The only exception is ``file://`` which is stripped from source unit names by the Host Filesystem
Loader.

When compiling locally you can use import remapping to replace the protocol and domain part with a
local path:

.. code-block:: bash

    solc :https://github.com/ethereum/dapp-bin=/usr/local/dapp-bin contract.sol

Note the leading ``:``, which is necessary when the remapping context is empty.
Otherwise the ``https:`` part would be interpreted by the compiler as the context.
