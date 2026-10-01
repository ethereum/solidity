.. index:: ! ethdebug, ! debug info, debugger

.. _ethdebug:

***************
ethdebug Output
***************

.. warning::

   ethdebug support is :ref:`experimental <experimental-mode>`.

The compiler can describe its output in the `ethdebug format <https://ethdebug.github.io/format/>`_, a JSON format for debugging information shared between compilers and debuggers.
The compiler targets version `0.1.0-preview.0 <https://github.com/ethdebug/format/tree/@ethdebug/format-web@0.1.0-preview.0>`_ of the format, which is still in development.
The format defines a `schema <https://github.com/ethdebug/format/tree/@ethdebug/format-web@0.1.0-preview.0/schemas>`_ for each kind of document.
This page lists the documents the compiler produces and describes the structure the compiler adds on top of the schemas, in particular how the type and pointer tables of the resources are keyed and named.
The options to request the outputs are documented in :ref:`compiler-api` and :ref:`commandline-compiler`.

Outputs
=======

+-------------------+-------------------------------------------+---------------------------------+---------------------------------------------+
| Document          | Standard JSON output                      | Command line                    | Schema                                      |
+===================+===========================================+=================================+=============================================+
| compilation       | ``ethdebug.compilation`` (global)         | ``--ethdebug-compilation``      | `ethdebug/format/materials/compilation`_    |
+-------------------+-------------------------------------------+---------------------------------+---------------------------------------------+
| resources         | ``ethdebug.resources`` (global)           | ``--ethdebug-resources``        | `ethdebug/format/info/resources`_           |
+-------------------+-------------------------------------------+---------------------------------+---------------------------------------------+
| creation program  | ``evm.bytecode.ethdebug`` (per contract)  | ``--ethdebug-program``          | `ethdebug/format/program`_                  |
+-------------------+-------------------------------------------+---------------------------------+---------------------------------------------+
| deployed program  | ``evm.deployedBytecode.ethdebug``         | ``--ethdebug-program-runtime``  | `ethdebug/format/program`_                  |
|                   | (per contract)                            |                                 |                                             |
+-------------------+-------------------------------------------+---------------------------------+---------------------------------------------+

.. _ethdebug/format/materials/compilation: https://github.com/ethdebug/format/blob/@ethdebug/format-web@0.1.0-preview.0/schemas/materials/compilation.schema.yaml
.. _ethdebug/format/info/resources: https://github.com/ethdebug/format/blob/@ethdebug/format-web@0.1.0-preview.0/schemas/info/resources.schema.yaml
.. _ethdebug/format/program: https://github.com/ethdebug/format/blob/@ethdebug/format-web@0.1.0-preview.0/schemas/program.schema.yaml

The compilation needs nothing but the sources.
The resources are derived from the analysis of the sources and the programs describe bytecode, so the latter can only be produced when compiling via IR.

The Compilation
===============

The compilation document identifies one run of the compiler and has an ``id`` derived from the sources, the ``compiler`` name and version and the list of ``sources``.
Every source carries its ``id``, ``path``, ``contents`` and ``language``.
A source ``id`` is a number unique within the compilation, and all source references in the other documents refer to sources by it.
The compiler currently uses the same number as the ``id`` of the source in the Standard JSON output and in :doc:`source mappings <internals/source_mappings>`.

The Resources
=============

The resources document carries two tables that the programs refer to, ``types`` and ``pointers``, and repeats the compilation, which ``ethdebug.compilation`` also provides on its own.
Both tables cover all contracts of the compilation.

.. code-block:: json

    {
        "compilation": {"id": "...", "compiler": {"name": "solc", "version": "..."}, "sources": ["..."]},
        "types": {"t_uint256": {"kind": "uint", "bits": 256}, "...": "..."},
        "pointers": {"t_struct$_Point_$6_storage": {"expect": ["slot"], "for": {"group": ["..."]}}, "...": "..."}
    }

Type Documents
--------------

The ``types`` table maps type identifiers to type documents (schema ``ethdebug/format/type``).
The compiler currently uses the identifiers its :ref:`storage layout <storage-layout-top-level>` output uses as well, such as ``t_uint256``, ``t_array$_t_uint8_$dyn_storage`` or ``t_struct$_Point_$6_storage``.
The only guarantee about the identifier is that it is a string that is unique within the table.
Its exact form is an implementation detail, but it is generally based on the name used in type definition.
When that name is not unique within the compilation, it may be disambiguated with additional information, such as the AST ID of the definition.

The table contains a document for the types of the state variables of every contract and of the parameters and return variables of every function and modifier compiled into it, including the ones inherited from base contracts, free functions and the internal functions of libraries.
Every type such a type is composed of has a document as well.
Types that only exist at compile time, such as literals, type names or the ``msg`` and ``abi`` objects, have no document.

Composite types reference their components by identifier, so that the table is closed:
an array, a mapping or a user-defined value type refers to its element, key, value or underlying type as ``{"type": {"id": "t_uint256"}}`` and a struct lists its members with their ``name`` and such a reference.
The parameters and the return values of a function type are tuple types, referenced in the same way.

Types defined in the source, that is structs, enums, contracts, user-defined value types and functions with a declaration, carry a ``definition`` with the ``name`` of the definition and its ``location`` in the source: the source ``id`` and the ``offset`` and ``length`` of the definition.
Like in source mappings, and as the schema of source ranges specifies, the offset and the length count bytes of the UTF-8 encoded source, not characters.

A type document does not depend on where a value is stored; that is what the pointer describes.
The variants of a reference type in different data locations therefore share one document: a ``string memory`` parameter and a ``string`` state variable both refer to the same entry.

Pointer Templates
-----------------

The ``pointers`` table maps type identifiers to pointer templates (schema ``ethdebug/format/pointer/template``): for every struct, array and mapping type that a state variable in storage has or is composed of, how a value of the type is laid out from a base slot.
The keys are the same identifiers the ``types`` table uses, so the template of a type sits next to its document.
Value types have no template.
Their values are single regions wherever they occur, and where a state variable's value starts is what the storage layout says; a debugger combines the two.

Every template expects ``slot``, the base slot of the value it describes.
The template of a mapping expects ``key`` as well, the key of the entry it locates: the value of a mapping is not at a fixed place, and a mapping nested in another type is represented by the region of its base slot, from which the mapping's template locates an entry once a key is bound.
A type nested in itself, through an array or through a mapping, references its own template, which a debugger follows as far as the data reaches: one element per list entry, one key per mapping level.

The regions a template produces are named relative to the value it describes:

- the members of a struct by their names,
- the elements of an array ``item``, with ``index`` being the variable the list iterates over,
- the length of a dynamic array ``length``,
- the parts of a ``bytes`` or ``string`` ``length-flag``, ``long-length`` and ``data``.

A template referencing the template of a member's or element's type prefixes the names that one produces with the member's name or ``item``, so that the ``x`` of the ``from`` member of a ``Line`` is ``from-x`` and that of an element of a ``Point[]`` is ``item-x``.
Since ethdebug identifiers must not start with ``$`` but Solidity identifiers may, a member name starting with ``$`` is prefixed with ``_``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    struct Point { uint8 x; uint8 y; }

    contract C {
        uint256 total;
        Point origin;
        mapping(address => uint256) balances;
    }

The resources of this contract contain these pointer templates.
``total`` has a value type and needs none; the storage layout puts it at slot 0.
The two members of ``Point`` share the base slot: a region's ``offset`` counts from the most significant byte of the slot, so the ``uint8`` in the least significant byte is at offset 31 and the one before it at offset 30.
A debugger instantiates the ``Point`` template with ``slot`` bound to 1, where the layout puts ``origin``, and the mapping's template with ``slot`` bound to 2 and ``key`` to the address of the entry it wants to see.

.. code-block:: json

    {
        "t_struct$_Point_$6_storage": {
            "expect": ["slot"],
            "for": {
                "group": [
                    {"location": "storage", "name": "x", "slot": "slot", "offset": "0x1f", "length": "0x01"},
                    {"location": "storage", "name": "y", "slot": "slot", "offset": "0x1e", "length": "0x01"}
                ]
            }
        },
        "t_mapping$_t_address_$_t_uint256_$": {
            "expect": ["slot", "key"],
            "for": {
                "location": "storage",
                "name": "value",
                "slot": {"$keccak256": [{"$wordsized": "key"}, {"$wordsized": "slot"}]}
            }
        }
    }

The shapes of the templates for the different kinds of types and the rules by which types become type documents are described in :ref:`ethdebug-internals`.

The Programs
============

A program (schema ``ethdebug/format/program``) describes one bytecode: the creation bytecode or the deployed bytecode of a contract.
It names the ``contract`` and the source range of its definition, states the ``environment`` the bytecode runs in, ``create`` or ``call``, and lists its ``instructions``.
Every instruction carries its byte ``offset`` in the bytecode, the ``operation`` with the ``mnemonic`` of the opcode and the ``arguments`` of a push, and, where the compiler knows it, a ``context`` with the source range of the code the instruction was generated from.
