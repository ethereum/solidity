:orphan:

.. index:: ! ethdebug, ! debug info

****************************
ethdebug Internal Debug Info
****************************

.. warning::

   ethdebug support and the interchange format described here are experimental.
   They may change before ethdebug output is stabilized.

.. note::

   This page is an initial draft and is deliberately kept out of the documentation TOC.
   It still mixes user-facing material with compiler internals; the two will later be split into separate documents.

The compiler emits public debug info using the
`ethdebug format <https://github.com/ethdebug/format>`_.
This page describes the internal debug info passed from the Solidity frontend through Yul before the public resources and contexts are emitted.
At a textual compiler boundary, the debug info is a JSON sidecar paired with the Yul source.

Terminology
===========

Several established terms are ambiguous in this context, so this page pairs each with a more specific replacement.

- *Metadata* elsewhere means the contract metadata embedded in bytecode.
  Data intended for debuggers is therefore called *debug info* on this page, never metadata.
- *Location* elsewhere names both positions in source text and the data locations of the Solidity language such as storage, memory, and calldata.
  This page uses *source range* for bytes in Solidity or Yul source text and a pointer's *region location* for EVM data in stack, storage, transient storage, memory, calldata, returndata, or code; the two are unrelated.
- *Pointer* elsewhere often means a Solidity storage or memory reference.
  Unless qualified that way, *pointer* on this page means an `ethdebug pointer <https://github.com/ethdebug/format/blob/main/schemas/pointer.schema.yaml>`_.

Fields introduced by this format follow these terms, for example ``declarationSourceRange``.
Existing compiler structures keep their established names, for example ``SourceLocation`` and the ``nativeLocation`` and ``originLocation`` fields of ``DebugData``.

.. note::

   The first implementation stage is intentionally narrower than the format specified here.
   It will leave variable updates empty, emit public variable contexts only for storage-backed variables, and not yet cover immutable, constant, or other variables whose materialized representation is not storage-backed.
   Yul optimization with ethdebug is also deferred.
   These are implementation staging limits, not sidecar-format constraints.

Design Requirements
===================

The sidecar is designed so that:

- A two-stage Solidity-to-Yul and Yul-to-bytecode compilation can produce the same bytecode and debug info as a one-stage compilation.
- A language frontend that targets Yul can supply semantic debug info independently of Solidity frontend objects.
- Source-language declaration identity and generated-Yul instance identity remain separate.
- Pointer region locations and variable phases describe EVM machine state and remain language-independent.

The Yul text carries ``@ast-id`` comments that identify source-language origins.
The sidecar carries the semantic records associated with those origins.
Both artifacts are required at a compiler-to-compiler text boundary.

Pipeline
========

The Solidity pipeline is designed to perform the following steps:

1. Solidity parsing assigns AST IDs, and analysis populates the existing type annotations on declarations.
2. When ethdebug debug info is requested, the frontend builds type and pointer resources and semantic records from type annotations, storage layout, and the ``IRVariable`` naming rules.
   This step uses analysis results and does not require IR or bytecode generation, so ``ethdebug.resources`` remains available after analysis.
3. IR generation emits Yul and, when requested, ``@ast-id`` comments.
4. ``CompilerStack`` attaches the table after parsing generated Yul into a ``YulStack``.
5. ``YulStack`` carries semantic records on Yul ``DebugData`` objects and retains unattached records in its side table.
6. When Yul is reparsed, the stack collects attached records into the table and reattaches them after parsing the printed Yul.
7. The output layer builds public type and pointer resources and program contexts from the table.

Debug-Info Dependency
=====================

Semantic debug-info transfer depends on ``@ast-id`` comments.
Accordingly, ``ethdebug`` depends on the ``ast-id`` ``--debug-info`` component.
CLI and Standard JSON input reject a debug-info selection containing ``ethdebug`` without ``ast-id``.
Selecting an ethdebug output does not modify the debug-info selection, just as ``--ir-optimized`` does not imply ``--optimize``.
Without ``ethdebug`` in the selection the program outputs are still produced from the assembly, but carry none of the semantic debug info the component adds; the resources and compilation outputs do not depend on it.

Core Structures
===============

.. note::

   Of the types named below, only ``langutil::DebugData`` exists in the compiler today.
   The ``SemanticDebugScope`` family and the ``semanticDebugScope`` field on ``DebugData`` are planned additions and will arrive with the implementation pull requests.

``langutil::DebugData`` is the debug payload carried by Yul AST nodes.
The sidecar envelope and variable records are compiler-specific.
Type documents and pointer documents follow the public `type <https://github.com/ethdebug/format/blob/main/schemas/type.schema.yaml>`_ and `pointer <https://github.com/ethdebug/format/blob/main/schemas/pointer.schema.yaml>`_ schemas, with one internal pointer expression for unresolved Yul locals.

.. list-table:: ``DebugData`` fields relevant to ethdebug
   :header-rows: 1
   :widths: 24 24 52

   * - Field
     - Type
     - Meaning
   * - ``nativeLocation``
     - ``SourceLocation``
     - Source range in the current Yul text.
   * - ``originLocation``
     - ``SourceLocation``
     - Source range in the source language.
   * - ``astID``
     - optional signed 64-bit integer
     - Source-language AST origin copied from ``@ast-id``.
   * - ``semanticDebugScope``
     - optional ``SemanticDebugScope`` pointer
     - Semantic variable payload attached to this Yul node.

``langutil::SemanticDebugScope`` describes the source node represented by an attached Yul node.
On a node that introduces a source scope it defines the scope's variables; on a finer-grained node it carries only variable updates.
The AST ID of the table entry identifies that source node.
Variables are recorded at their defining scope rather than at each use because a debugger needs every binding visible at a program point, including variables that the corresponding expression does not read.
The set of bindings visible at a program point follows from the chain of enclosing scope records, so nested scopes do not duplicate enclosing variables.
Use sites are also unstable under inlining, common-subexpression elimination, and dead-code removal.
The scope record therefore carries declaration identity, type, and initial recovery information once; ``variableUpdates`` supplies sparse, statement-level changes only when a variable's phase or pointer changes within that scope.

.. list-table:: ``SemanticDebugScope``
   :header-rows: 1
   :widths: 24 24 52

   * - Field
     - Type
     - Meaning
   * - ``variableDefinitions``
     - array of ``SemanticDebugVariable``
     - Bindings introduced by the scope, in source order.
       A variable appears in exactly one scope record; nested records do not repeat enclosing bindings.
   * - ``variableUpdates``
     - array of ``SemanticDebugVariableUpdate``
     - Phase or pointer changes taking effect at the attached node - see Variable Updates.

``SemanticDebugVariable`` stores a source binding together with its type, recovery phase, and pointer when materialized.

.. list-table:: ``SemanticDebugVariable``
   :header-rows: 1
   :widths: 27 25 48

   * - Field
     - Type
     - Meaning
   * - ``identifier``
     - optional string
     - Source-language identifier.
       It is absent for unnamed variables such as unnamed Solidity return parameters.
   * - ``declarationASTID``
     - optional signed 64-bit integer
     - Identity of the source-language declaration.
       Synthetic bindings may omit it.
   * - ``declarationSourceRange``
     - optional ``SourceLocation``
     - Source range of the declaration.
   * - ``typeID``
     - optional string
     - Key of the variable's document in ``ethdebug.resources.types``.
       The sidecar carries the same table in its top-level ``resources`` object.
   * - ``phase``
     - ``SemanticDebugVariablePhase``
     - Whether the value is materialized, computable, or unavailable - see Variable Phases and Pointers.
   * - ``pointer``
     - optional ``SemanticDebugPointer``
     - Pointer expression resolving a materialized value.

Field names are identical in the JSON form and the compiler structures.
Every AST ID field accepts values from ``-2**63`` through ``2**63 - 1``, matching ``int64_t``.

Pointers for persistent and transient state-variable declarations are derived from the storage layout after analysis.
They contain literal slots and offsets or expressions over template parameters such as mapping keys.
Producing the public type and pointer resource tables therefore does not require code generation.

Local storage references are different: their base slot is a runtime value.
For example, after ``uint[] storage ref = condition ? a : b``, the pointer for ``ref`` starts at the generated Yul local holding the selected slot rather than at a statically known slot.
Stack values, memory references, and such storage references can use the internal ``$$yulLocal`` expression described below until Yul-to-EVM code generation assigns their concrete representation.
The expression must be resolved or removed before public emission.

Scope Attachment
----------------

Semantic scope data belongs to the Yul node that introduces the corresponding scope.

- Function and modifier records attach to the generated Yul ``FunctionDefinition``.
- A source block lowered to a distinct Yul block attaches to that ``Block``.
- A conditional, loop clause, or catch clause that introduces a source scope attaches its payload to the generated block representing that scope.
- Variables visible at contract and file level are collected in the record attached to the generated object's top-level Yul ``Block``.
  They do not remain side-table-only merely because their declarations have no distinct Yul node.

Every Yul AST node kind carries ``DebugData``, including names in parameter and return lists, so a scope payload can attach to any node.
The producer is responsible for selecting the node that semantically owns a scope.
The side table retains records while they are detached during parsing or transformation and any record for which no surviving attachment exists.

Variable Phases and Pointers
============================

``SemanticDebugVariablePhase`` describes whether a value can be recovered at a program point.
It does not duplicate the physical location already present in an ethdebug pointer.

.. list-table:: Variable phases
   :header-rows: 1
   :widths: 25 75

   * - Phase
     - Meaning
   * - ``materialized``
     - The value has a concrete representation described by ``pointer``.
   * - ``computed``
     - The value has no region because the program recomputes it where it is used.
       The format carries no recovery recipe, so a debugger reports the value as unavailable; the phase records that recomputation would be possible in principle, unlike ``optimized-out``.
   * - ``optimized-out``
     - No recoverable representation is available at this program point.

.. note::

   Describing computed values with value expressions, similar to DWARF expression locations, would require an ethdebug schema extension: pointers currently describe regions of EVM data and cannot yield a value directly.

A materialized variable has a pointer; a computed or optimized-out variable does not.
For a materialized value, the pointer schema's ``location`` field says whether each region is on the stack or in storage, transient storage, memory, calldata, returndata, or code.
Solidity immutables use a memory pointer while creation code initializes them and a code pointer when read from deployed code.
A constant folded to an immediate may be described by code bytes, while a constant expression that must execute has the ``computed`` phase.

``SemanticDebugPointer`` serializes to an `ethdebug pointer`_ document.
Regions, groups, lists, conditionals, scopes, and template references use the corresponding structural forms from that schema.
The pointer schema's ``define``/``in`` form binds intermediate expressions used by the pointer itself, for example an array length or the start of its data.
If one such binding depends on another, the producer nests ``define``/``in`` objects in dependency order.
These expression bindings are unrelated to Yul lexical scopes and to ``SemanticDebugScope`` records.
A root pointer with externally bound parameters, such as mapping keys, uses the schema's template form ``{"expect": [...], "for": ...}``.

Expressions use the `ethdebug pointer-expression grammar <https://ethdebug.github.io/format/docs/core-schemas/pointers/expressions/>`_.
Literals are non-negative JSON numbers or ``0x``-prefixed strings; bound variables are identifier strings.
The remaining forms are ``"$wordsize"`` and the schema's single-key objects for lookups, reads, arithmetic, hashing, concatenation, and resizing.

Expressions internal to the compiler use a ``$$`` prefix, which cannot clash with the schema's own ``$`` forms.
The internal expression ``{"$$yulLocal": ...}`` names a generated Yul local whose stack depth has not yet been assigned.
It has no public schema form and must be resolved or removed before emission; emission rejects any expression still carrying the ``$$`` prefix.

Reusable pointers are published as resource templates.
The sidecar carries them in the ``pointers`` table of its ``resources`` object, keyed by producer-defined name identifier and mirroring `ethdebug.resources.pointers <https://github.com/ethdebug/format/blob/main/schemas/info/resources.schema.yaml>`_.
A variable whose location is described by a template carries the pointer schema's ``template`` reference to the table entry, so pointer templates are referenced the same way as types.
Both resource tables are derived from analysis results; producing them does not wait for Yul-to-EVM code generation.
Public emission copies the tables into ``ethdebug.resources`` unchanged.
A closed template can also be instantiated in a program context; a template with external parameters, such as mapping keys, remains a resource until the consumer supplies those parameters.
A ``template`` reference must resolve against a `local templates block <https://ethdebug.github.io/format/spec/pointer/collection/templates>`_ in the same pointer document or against the sidecar's ``resources.pointers`` table; deserialization rejects a reference to any other name.

Phase Changes
-------------

After attachment or reparse, the producer checks all free Yul variable names used by a pointer.
If any required name is absent from the current Yul object, the phase becomes ``optimized-out`` and the pointer is removed.
The check excludes names bound by pointer ``define`` objects, list indices, and template parameters.

Transformations that preserve a value must rewrite its pointer expressions to the new Yul names or machine regions.
Transformations that clone or specialize code must clone the scope-instance debug info as well.
If a pass cannot describe a surviving value soundly, it must use ``optimized-out`` rather than retain a stale pointer.

Variable Updates
~~~~~~~~~~~~~~~~

A single per-scope phase and pointer cannot describe a value throughout its lifetime.
``SSATransform`` gives one source variable a different generated name at each assignment, while ``Rematerialiser`` replaces a materialized value with recomputation at individual uses.
For these, an entry carries ``variableUpdates`` - the analogue of an ``llvm.dbg.value`` intrinsic:

.. list-table:: ``SemanticDebugVariableUpdate`` fields
   :header-rows: 1
   :widths: 28 24 48

   * - Field
     - Type
     - Meaning
   * - ``variableASTID``
     - signed 64-bit integer
     - Declaration AST ID of the variable being updated.
   * - ``phase``
     - optional ``SemanticDebugVariablePhase``
     - Replacement phase valid from the carrying node onward.
       It is omitted when only the pointer changes.
   * - ``pointer``
     - optional ``SemanticDebugPointer``
     - Replacement pointer for a materialized value.
       A change to ``computed`` or ``optimized-out`` clears the previous pointer.

An update takes effect at the Yul node its payload is attached to and holds until the next update for the same variable within the scope, or the scope's end.
Scope records define variables; statement-level records update them.
A ``Drop`` changes the phase to ``optimized-out``.
Only passes whose effect the scope's initial phase and pointer cannot express need to produce updates.

Deleting a Yul node that carries updates does not merely lose detail: the variable's previous binding would incorrectly appear current for the rest of the scope.
A pass that removes such a node must reattach the updates to the node that takes its place or emit a ``Drop`` update for the affected variables at that program point.

Most optimizer changes are pointer rewrites or phase changes rather than physical relocation.
An SSA rename keeps the value ``materialized`` but changes the Yul local named by its pointer; rematerialization changes the phase to ``computed``.
``StackToMemoryMover``, including its use by ``StackLimitEvader``, is an actual pointer-region change: its pointer changes from stack to memory while its phase remains ``materialized``.
By contrast, ``LoadResolver`` creates or reuses a stack copy of a loaded value but does not move the source variable out of storage, so the source variable's storage pointer does not change.

Optimizer Update Rules
----------------------

Every transformation that rewrites Yul with ethdebug enabled must declare how it maintains the semantic side table.
The pass itself must determine whether a value survived unchanged, moved, merged with another value, became computable, or disappeared; a generic pass driver cannot infer that reliably.
A pass with no declared strategy is treated as ``Drop``, so an undeclared pass degrades debug info instead of producing stale pointers.

.. list-table:: Debug-info update strategies
   :header-rows: 1
   :widths: 14 48 38

   * - Strategy
     - Obligation
     - Passes
   * - ``Preserve``
     - Copy the debug entry unchanged.
       Valid only when the pass changes neither the phase nor the names or regions read by a pointer.
     - ``ForLoopInitRewriter``, ``ForLoopConditionIntoBody``, ``ForLoopConditionOutOfBody``, ``VarDeclInitializer``, ``FunctionHoister``, ``FunctionGrouper``, ``ConditionalUnsimplifier``
   * - ``Merge``
     - Keep the surviving value's entry when two values or two blocks collapse into one.
       Record the discarded declarations against the same program point so both source names remain inspectable.
     - ``CommonSubexpressionEliminator``, ``ExpressionJoiner``, ``ExpressionSimplifier``, ``ControlFlowSimplifier``, ``StructuralSimplifier``, ``BlockFlattener``, ``EquivalentFunctionCombiner``, ``LoadResolver``
   * - ``Remap``
     - Rewrite the pointer when the pass renames or physically moves a materialized value.
       Renaming passes rewrite the Yul names a pointer reads; because a statically addressed pointer holds no Yul name, renaming cannot affect it.
       Spilling passes move where the value is found, so the pointer changes from a stack to a memory region; the data itself is not relocated by the debug info.
     - ``Disambiguator``, ``NameSimplifier``, ``VarNameCleaner``, ``SSATransform``, ``SSAReverser``, ``ExpressionSplitter``, ``LoopInvariantCodeMotion``, ``StackToMemoryMover``, ``StackCompressor``, ``StackLimitEvader``
   * - ``Recompute``
     - Change the phase to ``computed`` from the affected program point and remove the stale materialized pointer.
     - ``LiteralRematerialiser``, ``Rematerialiser``
   * - ``Clone``
     - Duplicate the debug entry for each generated copy and give each copy its own instance discriminator.
       Without the instance discriminator the copies overwrite each other in the table.
     - ``FullInliner``, ``FunctionSpecializer``, ``ExpressionInliner``
   * - ``Drop``
     - Set the phase to ``optimized-out`` and remove the pointer.
       The declaration, type, and source range are retained so the variable is still reported as belonging to the scope.
     - ``DeadCodeEliminator``, ``UnusedPruner``, ``UnusedAssignEliminator``, ``UnusedStoreEliminator``, ``EqualStoreEliminator``, ``CircularReferencesPruner``, ``UnusedFunctionParameterPruner``

``SSATransform`` and ``Rematerialiser`` cannot be expressed as whole-scope updates, because the pointer or phase changes between program points.
They must emit variable updates instead - see `Variable Updates`_.
Substituting a variable use by its defining expression can leave the variable itself unused and later pruned.
The value is then still recoverable by evaluating that expression, so its phase becomes ``computed`` rather than ``optimized-out``.

A pass that cannot meet its obligation for a particular value falls back to ``Drop`` for that value alone, not for the whole scope.
Marking a value unavailable is always sound, while keeping a pointer that no longer describes the value is not.

Identity and Optimizer Cloning
==============================

``astID`` records the source-language origin.
Specialization and cloning can produce several Yul scopes with the same origin but different variable phases or pointers.

The table therefore uses ``(astID, instance)`` as its key:

- ``astID`` is a signed 64-bit integer that identifies the source-language node and is preserved by ``@ast-id``.
- ``instance`` is a signed 64-bit integer in the range ``0`` through ``2**63 - 1`` that distinguishes generated Yul scope instances sharing the same source origin.
  It defaults to ``0``.

An entry that has not been cloned uses instance ``0``.
In Yul text, the instance is carried by a dedicated ``@ast-id-instance`` annotation emitted next to ``@ast-id`` and present only when the instance is not ``0``.
The ``@ast-id`` annotation itself is unchanged, so uncloned code keeps its current form and existing consumers are unaffected.
An ``@ast-id-instance`` annotation without an accompanying ``@ast-id`` on the same node is malformed and is ignored.
Optimizer passes that clone or specialize functions assign fresh instance values and emit the annotation on each clone.

Serialization
=============

One shared component provides both serialization and deserialization, so the writer and the reader cannot drift apart.
Serialization includes the resource tables, pointer expressions, variable updates, and scope records that have no Yul attachment.
All integer fields are native JSON numbers; the reader decodes the full 64-bit ranges exactly.
String-encoded numbers occur only inside pointer expressions, where the ethdebug expression grammar governs them.
Declaration and type-definition ranges use the ethdebug `source range <https://github.com/ethdebug/format/blob/main/schemas/materials/source-range.schema.yaml>`_ shape.
Source IDs are opaque to the Yul stage.
The producer uses the same source IDs in the sidecar as in the accompanying ``ethdebug.compilation`` record, and the Yul stage transports them unchanged.
Each source record in ``ethdebug.compilation`` maps its ID to the source path and contents, so a path is not used as an ID.

.. list-table:: Top-level JSON object
   :header-rows: 1
   :widths: 22 22 56

   * - Field
     - Type
     - Meaning
   * - ``format``
     - string
     - ``solidity-ethdebug-semantic-data``.
   * - ``version``
     - unsigned 32-bit integer
     - Format version in the range ``0`` to ``2**32 - 1``, currently ``1``.
   * - ``contractName``
     - optional string
     - Source-language contract name used by the public ethdebug program object when the generated Yul object has a different name.
   * - ``resources``
     - optional object
     - The ``types`` and ``pointers`` members of the `ethdebug resource tables <https://github.com/ethdebug/format/blob/main/schemas/info/resources.schema.yaml>`_, keyed by producer-defined IDs.
       Variables refer to type entries through ``typeID`` and to pointer templates through ``template`` references.
       Public emission copies both tables into ``ethdebug.resources``.
   * - ``scopes``
     - object
     - Two-level map from the decimal string form of ``astID`` to the decimal string form of ``instance`` to a ``SemanticDebugScope``.
       Instance ``0`` is written explicitly.

Readers reject an unknown format, an unsupported version, malformed sidecar fields, and keys that do not parse as decimal integers in the documented ranges.
Writers emit deterministic output by ordering both key levels in ascending numeric order.

Compiler Interfaces
-------------------

Standard JSON uses these fields:

- Solidity output ``contracts[<source>][<contract>].ir`` contains the Yul text.
- Solidity output ``contracts[<source>][<contract>].irEthdebug`` contains its semantic sidecar.
- Yul input ``auxiliaryInput.ethdebug`` supplies the sidecar for the Yul source.
- Yul output ``irEthdebug`` emits the updated sidecar again.

The command-line interface uses these options:

- ``--ir`` and ``--ir-ethdebug`` emit the Yul text and sidecar.
- ``--strict-assembly <source.yul> --ethdebug-input <sidecar.json>`` supplies the sidecar for the Yul input.
  Assembler modes accept exactly one input file, so no syntax pairing sidecars with multiple inputs is defined.

The sidecar is both compiler input and compiler output, allowing the same debug info to cross a textual Yul boundary.

.. note::

   For implementation testing, ``EthdebugTest`` will add ``Contract.semantic`` as a direct view of the serialized sidecar.

Required Variable Coverage
==========================

Semantic debug info must cover every binding visible to source-level debugging:

- Named and unnamed function parameters and return parameters are included.
- Modifier parameters and local variables in ordinary blocks are included.
- Variables introduced by ``for``, ``if``, ``try``, success, ``catch``, error, and panic clauses are included in the precise scope where they become visible.
- State variables in persistent and transient storage are included.
- File-level constants are included.
- Every visible import alias is a separate binding with its alias identifier but shares the declaration identity and value description of the imported constant.
- Synthetic language bindings such as ``this``, ``super``, and ``msg`` are included even when there is no ``VariableDeclaration`` AST node.
- Values in memory, calldata, returndata, code, and computed form are included when they are observable.

ethdebug variable identifiers are optional.
An unnamed Solidity return parameter is therefore emitted with declaration, type, and pointer information but without ``identifier``.
Its declaration order and AST identity still map it to the corresponding Solidity return slot and generated ``IRVariable`` stack slots.
