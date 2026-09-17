.. index:: ! ethdebug, ! debug info, debugger

.. _ethdebug:

***************
ethdebug Output
***************

.. warning::

   ethdebug support is experimental.
   The outputs described here can only be requested together with the ``experimental`` setting and may change before they are stabilized.

The compiler can describe its output in the `ethdebug format <https://ethdebug.github.io/format/>`_, a JSON format for debugging information shared between compilers and debuggers.
The format defines a `schema <https://github.com/ethdebug/format/tree/main/schemas>`_ for each kind of document.
This page lists the documents the compiler produces and describes the structure the compiler adds on top of the schemas, in particular how the type and pointer tables of the resources are keyed and named.
The options to request the outputs are documented in :ref:`compiler-api` and :ref:`commandline-compiler`.

Outputs
=======

+-------------------+-------------------------------------------+---------------------------------+-------------------------------------------+
| Document          | Standard JSON output                      | Command line                    | Schema                                    |
+===================+===========================================+=================================+===========================================+
| compilation       | ``ethdebug.compilation`` (global)         | ``--ethdebug-compilation``      | ``ethdebug/format/materials/compilation`` |
+-------------------+-------------------------------------------+---------------------------------+-------------------------------------------+
| resources         | ``ethdebug.resources`` (global)           | ``--ethdebug-resources``        | ``ethdebug/format/info/resources``        |
+-------------------+-------------------------------------------+---------------------------------+-------------------------------------------+
| creation program  | ``evm.bytecode.ethdebug`` (per contract)  | ``--ethdebug-program``          | ``ethdebug/format/program``               |
+-------------------+-------------------------------------------+---------------------------------+-------------------------------------------+
| runtime program   | ``evm.deployedBytecode.ethdebug``         | ``--ethdebug-program-runtime``  | ``ethdebug/format/program``               |
|                   | (per contract)                            |                                 |                                           |
+-------------------+-------------------------------------------+---------------------------------+-------------------------------------------+

The compilation and the resources are available as soon as the analysis of the sources succeeded.
The programs describe bytecode and can only be produced when compiling via IR.

The Compilation
===============

The compilation document identifies one run of the compiler: an ``id`` derived from the sources, the ``compiler`` name and version and the list of ``sources``.
Every source carries its ``id``, ``path``, ``contents`` and ``language``.
The source ``id`` is the index of the source unit in the compilation, the same number the ``id`` of a source in the Standard JSON output and the source mappings use.
All source references in the other documents refer to sources by that ``id``.

The Resources
=============

The resources document repeats the compilation and adds two tables that the programs refer to: ``types`` and ``pointers``.
Both tables cover all contracts of the compilation.

.. code-block:: json

    {
        "compilation": { "id": "...", "compiler": { "name": "solc", "version": "..." }, "sources": [ "..." ] },
        "types": { "t_uint256": { "kind": "uint", "bits": 256 }, "...": "..." },
        "pointers": { "storage_16_8": { "expect": [], "for": { "location": "storage", "name": "total", "slot": "0x00" } }, "...": "..." }
    }

Type Documents
--------------

The ``types`` table maps type identifiers to type documents (schema ``ethdebug/format/type``).
The identifiers are the compiler's canonical type identifiers, which the ``storageLayout`` output uses as well: ``t_uint256``, ``t_address_payable``, ``t_array$_t_uint8_$dyn_storage``, ``t_mapping$_t_address_$_t_uint256_$`` or ``t_struct$_Point_$6_storage``, where the number in the identifier of a user-defined type is the ID of its definition in the AST.

The table contains a document for the types of the state variables of every contract and of the parameters and return variables of every function and modifier compiled into it, including the ones inherited from base contracts, free functions and the internal functions of libraries.
Every type such a type is composed of has a document as well.
Types that only exist at compile time, such as literals, type names or the ``msg`` and ``abi`` objects, have no document.

Composed types reference their components by identifier, so that the table is closed:
an array, a mapping or a user-defined value type refers to its element, key, value or underlying type as ``{"type": {"id": "t_uint256"}}`` and a struct lists its members with their ``name`` and such a reference.
Function types are the exception: their parameters and return values are described as tuples inline.

Types defined in the source, that is structs, enums, contracts, user-defined value types and functions with a declaration, carry a ``definition`` with the ``name`` of the definition and its ``location`` in the source: the source ``id`` and the byte ``offset`` and ``length`` of the definition.

The data location of a reference type is part of its identifier but not of its document, since the schema describes types independently of where their values are stored.
``t_string_storage`` and ``t_string_memory_ptr`` are therefore separate entries with the same document.
The members of a struct carry the types they have in the struct's data location.

Pointer Templates
-----------------

The ``pointers`` table maps names to pointer templates (schema ``ethdebug/format/pointer/template``), one for every state variable in storage and transient storage of every contract in the compilation.
Constants and immutables occupy no storage and have no template.

A template is named ``storage_<contract>_<variable>`` or ``transient_<contract>_<variable>``, where ``<contract>`` is the AST ID of the contract and ``<variable>`` the AST ID of the variable declaration, as the ``ast`` output lists them.
An inherited variable has one template per contract that inherits it, since its slot depends on the contract.

The ``expect`` list names the parameters of the template.
A variable whose type contains a mapping expects one parameter per mapping key, ``key`` for the outermost mapping and ``key1``, ``key2`` and so on for the mappings nested in its values.
A debugger instantiates such a template by binding the keys of the entry it wants to inspect.
All other templates expect no parameters and describe the variable as is.

The ``for`` pointer describes the location of the variable's value with regions named after the variable:

- ``<variable>`` is the region holding the value of a variable of value type, or the data of a ``bytes`` or ``string`` variable.
- ``<variable>-<member>`` are the regions of the members of a struct.
- ``<variable>-item`` is the region of an element of an array, with ``<variable>-index`` being the index the list iterates over.
- ``<variable>-length`` and ``<variable>-data`` hold the length and the position of the data of a dynamic array, and ``<variable>-length-flag`` and ``<variable>-long-length`` are the parts of the length encoding of ``bytes`` and ``string``.

These names nest, so that the ``x`` member of the ``origin`` struct below is ``origin-x`` and an element of an array member ``items`` of that struct would be ``origin-items-item``.
Since ethdebug identifiers must not start with ``$`` but Solidity identifiers may, a name starting with ``$`` is prefixed with ``_``.

.. code-block:: solidity

    // SPDX-License-Identifier: GPL-3.0
    pragma solidity >=0.6.0 <0.9.0;

    struct Point { uint8 x; uint8 y; }

    contract C {
        uint256 total;
        Point origin;
        mapping(address => uint256) balances;
    }

The resources of this contract contain these pointer templates, keyed by the AST IDs of ``C`` and of the three variables:

.. code-block:: json

    {
        "storage_16_8": {
            "expect": [],
            "for": {"location": "storage", "name": "total", "slot": "0x00"}
        },
        "storage_16_11": {
            "expect": [],
            "for": {
                "group": [
                    {"location": "storage", "name": "origin-x", "slot": "0x01", "length": "0x01"},
                    {"location": "storage", "name": "origin-y", "slot": "0x01", "offset": "0x01", "length": "0x01"}
                ]
            }
        },
        "storage_16_15": {
            "expect": ["key"],
            "for": {
                "location": "storage",
                "name": "balances",
                "slot": {"$keccak256": [{"$wordsized": "key"}, {"$wordsized": "0x02"}]}
            }
        }
    }

The shapes of the pointers for the different kinds of types and the rules by which types become type documents are described in :ref:`ethdebug-internals`.

The Programs
============

A program (schema ``ethdebug/format/program``) describes one bytecode: the creation bytecode or the runtime bytecode of a contract.
It names the ``contract`` and the source range of its definition, states the ``environment`` the bytecode runs in, ``create`` or ``call``, and lists its ``instructions``.
Every instruction carries its byte ``offset`` in the bytecode, the ``operation`` with the ``mnemonic`` of the opcode and the ``arguments`` of a push, and, where the compiler knows it, a ``context`` with the source range of the code the instruction was generated from.
