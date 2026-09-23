.. index:: ! ethdebug; implementation

.. _ethdebug-internals:

***********************
ethdebug Implementation
***********************

This page is aimed at compiler developers.
It describes how the compiler derives the :ref:`ethdebug output <ethdebug>` from its analysis results: how each type of the language maps to the type schema and what the pointers of the state variables look like for each kind of type and location.
The debug info that is passed from the Solidity frontend through Yul is specified in :doc:`ethdebug_internal_debug_info`.

The documents are modelled as data structures in ``libevmasm/EthdebugSchema.h``, one per schema, and serialized to JSON from there.
The constructors of the parts assert the constraints of the schemas, so that the compiler cannot assemble a document that does not serialize.
The frontend side, ``libsolidity/interface/Ethdebug.cpp``, builds the type documents and the pointer templates of a contract from its AST and types.

Type Documents
==============

Which Types Get a Document
--------------------------

The resources of a contract contain a document for

- the types of its state variables in storage and transient storage,
- the types of the parameters and return variables of every function and modifier compiled into the contract: the ones it defines, the ones it inherits, the free functions of the source units it references and the internal functions of the libraries in those source units,
- and, transitively, every type these types are composed of.

The resources of a compilation are the union of the resources of its contracts.
Types are keyed by ``Type::identifier()``, so a type shared between contracts has one document.

A type that only exists at compile time has no document, and neither has a type composed of one.
This concerns literals, type names, modifiers, magic types, modules, inaccessible dynamic types and function types other than internal and external ones.
To terminate on recursive types, a placeholder is registered before the components of a type are visited and replaced by the document afterwards.

The Mapping
-----------

.. list-table::
   :header-rows: 1

   * - Solidity type
     - ``Type::Category``
     - ethdebug kind
     - Properties
   * - ``uintN``, ``intN``
     - Integer
     - uint, int
     - ``bits``
   * - ``bool``
     - Bool
     - bool
     -
   * - ``address``, ``address payable``
     - Address
     - address
     - ``payable``
   * - ``bytesN``
     - FixedBytes
     - bytes
     - ``size``
   * - ``bytes``
     - Array
     - bytes
     - no ``size``
   * - ``string``
     - Array
     - string
     -
   * - ``fixedNxM``, ``ufixedNxM``
     - FixedPoint
     - fixed, ufixed
     - ``bits``, ``places``
   * - contract, library, interface
     - Contract
     - contract
     - ``library`` or ``interface`` when the contract is one, ``payable``, ``definition``
   * - enum
     - Enum
     - enum
     - ``values`` in declaration order, ``definition``
   * - ``type U is V``
     - UserDefinedValueType
     - alias
     - ``contains`` referencing ``V``, ``definition``
   * - ``T[N]``
     - Array
     - array
     - ``contains`` referencing ``T``, ``count``
   * - ``T[]``
     - Array
     - array
     - ``contains`` referencing ``T``, no ``count``
   * - array slice
     - ArraySlice
     - array
     - like the dynamic array it views
   * - ``mapping(K => V)``
     - Mapping
     - mapping
     - ``contains.key`` referencing ``K``, ``contains.value`` referencing ``V``
   * - struct
     - Struct
     - struct
     - ``contains``: the members with ``name`` and a reference to their type, ``definition``
   * - tuple
     - Tuple
     - tuple
     - ``contains``: the components; only inline, as the parameters of function types
   * - internal or external function type
     - Function
     - function
     - ``internal`` or ``external``, ``contains.parameters`` and ``contains.returns`` as inline tuples (no ``returns`` without return values), ``definition`` when declared

References to component types are ``{"type": {"id": <identifier>}}``.
The data location of a reference type is not represented; the located variants of a type are separate entries with the same document.
The members of a struct are taken from ``StructType::members()`` and therefore carry the types they have in the struct's data location, e.g. a ``string`` member of a storage struct references ``t_string_storage``.

A ``definition`` carries the ``name`` of the declaration, unless it is empty, and its source range as the ``location``.
The source ``id`` of the location is the index of the source unit in ``CompilerStack::sourceIndices()``, which is also what the compilation record and the source mappings use.
Definitions without a name and a known location are omitted.

State Variable Pointers
=======================

The pointer table describes the storage types of the state variables, one template per struct, array and mapping type that a state variable has or is composed of, keyed by the type identifier and expecting the base slot of a value as ``slot``.
Value types have no template; a value of value type is a single region wherever it occurs.
Where a state variable's value starts comes from ``ContractType::linearizedStateVariables()``, which is what the storage layout output reports as well, and is not part of the resources.

``PointerTemplateRegistry`` builds the templates from the types, registering the template of a type when it is first needed and of every struct, array and mapping type it composes.
A type nested in itself, through an array or a mapping, references its own template, which is resolved lazily by the consumer; there is no depth limit and nothing is left undecomposed.
The kinds of types are laid out as follows, where ``slot`` stands for the expression addressing the base slot of the value at hand: the expected variable for the template's own value and a computed expression for a part of it.

Value types
    A single region ``{"location": "storage", "name": <name>, "slot": slot}``.
    A value narrower than a word carries its ``length`` in bytes and its ``offset`` in the slot.
    The offset counts from the most significant byte of the slot, as the pointer format's segment addressing does and the reference implementation reads it, while the storage layout packs values from the least significant byte: a value of *n* bytes at layout offset *o* starts at byte 32 - *o* - *n*, so a ``uint8`` alone in its slot is at offset 31 and an ``address`` packed after two bytes is at offset 10.
    An offset of zero is omitted, and so is a ``length`` of a whole word, since a region without one covers the rest of its slot.

Structs
    A ``group`` of the pointers of the members, each built from the type the member has in storage at ``slot`` advanced by the member's slot offset and, when packed, at its byte offset.
    A member of value type is a region named after the member; a member of a type that has a template references that template with the member's slot bound, renaming the regions it produces with the member's name as a prefix.

Static arrays ``T[N]``
    A ``list`` with ``count`` ``N`` and the index variable ``index``, whose element ``item`` is:

    - for a value type narrower than a word, packed *k* to a slot from the least significant byte, the region at slot ``slot + index / k``, offset ``$wordsize - (index % k + 1) * size`` and length ``size``,
    - for another value type the region at ``slot + index * slots``, with ``slots`` being the storage size of the element type, omitted when it is one,
    - for a type with a template a reference to it with that slot bound, its regions prefixed with ``item``.

Dynamic arrays ``T[]``
    A ``group`` of the region ``length`` at ``slot`` and a scope defining ``data`` as ``keccak256(wordsized(slot))``, in which the elements are the list of a static array with ``count`` ``$read(length)`` starting at ``data``.

``bytes`` and ``string``
    The compact encoding keeps values shorter than 32 bytes in the slot itself, with twice the length in the lowest byte, and longer values at ``keccak256(slot)``, with twice the length plus one in the slot.
    The pointer is a ``group`` of the region ``length-flag``, the lowest byte of the slot, and a conditional on ``($read(length-flag) + 1) % 2``:

    - the short case defines ``length`` as ``$read(length-flag) / 2`` for the region ``data`` in the slot with that length,
    - the long case is a ``group`` of the region ``long-length`` at the slot and a scope defining ``length`` as ``($read(long-length) - 1) / 2`` and ``start`` as ``keccak256(wordsized(slot))`` for the region ``data`` at ``start`` with that length.

Mappings ``mapping(K => V)``
    The template expects ``key`` besides ``slot``; the entry's value lives at ``keccak256(wordsized(key), wordsized(slot))`` and is described like a member called ``value``: a region for a value type, a reference to the value type's template otherwise.
    Keys of value type are hashed as full words; ``bytes`` and ``string`` keys are hashed unpadded.
    A mapping that is the member of a struct, the element of an array or the value of another mapping is the region of its base slot, named after the member, ``item`` or ``value``, since its entries are not at any fixed place: a consumer instantiates the mapping's template with that slot and the next key.
    This is also how a type nested in itself through a mapping is described, one key per level, rather than by a template expecting a fixed list of keys.

Transient storage
    Only value types can be declared ``transient``, so transient storage adds no templates; a transient state variable is a single region with the location ``transient``.

Slot arithmetic on literal slots is folded into the literals, so that the members of a struct at slot 3 are addressed as ``0x03``, ``0x04`` and so on rather than as sums.
The test cases under ``test/libsolidity/ethdebugTests/resources/`` show the complete type and pointer tables for each of these layouts and are the reference for their exact shape.
