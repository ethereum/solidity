.. index:: standard.json file format

.. _STANDARD_JSON_FORMAT:

*********************************************************
Solidity Compiler standard.json File Format Specification
*********************************************************

InputsAndSettings
-----------------

This object contains all necessary fields to configure the compiler for processing the
inputs with their given settings and output selections.

- ``language: 'Solidity' | 'Yul'``: Declares whether or not the input sources are solidity or yul sources.
- ``sources: Sources``
- ``auxiliaryInput?``
- ``settings: Settings``

language
--------

This is a string denoting the language the given sources are in. This should usually
be "solidity".

.. _Sources:

Sources
-------

`Sources` is an object with each member name being the file path of a source
and its value is of type `Source`, defining the source properties, such as contents,
but optionally also more.

Source
------

Members:

- ``keccak256: String?`` this is an optional hash of the input's source code, used for verification
- ``content: String?`` contains the source code of for this file path
- ``urls: String[]?`` Contains a list of URLs to fetch instead. Use only when ``content`` is omitted.
- ``filetype: String`` of type string (such as: ``"solidity"``)

Note: `content` can be omitted if `urls` is specified, only either `content` or `urls`
must be present.

auxiliaryInput
--------------

`auxiliaryInput` is an object with each member name being a valid `h256` hash
and its value a SMTLib response.

TODO: better doc

Settings
--------

- ``parserErrorRecovery: bool = false``
- ``evmVersion: EVMVersion``
- ``debug: Debug``
- ``remappings: RemappingString``
- ``outputSelection: OutputSelection``
- ``optimizer: OptimizerSettings``
- ``libraries: LibrariesSettings``
- ``metadata: Metadata``
    - ``bytecodeHash: ipfs | bzzr1 | null``

EVMVersion
----------

This is a string matching a EVM version the compiler should target when compiling.

Debug
-----

- ``revertStrings: 'default' | 'strip' | 'debug' | 'verboseDebug'``

RemappingString
---------------

RemappingString is a string with a special format.

The format is equal to the format used in the command line input.

OptimizerSettings
-----------------

- `enabled: bool`
- `runs: int`
- `details: OptimizerSettingDetails`

OptimizerSettingDetails
-----------------------

- ``peephole: bool = false``
- ``jumpdestRemover: bool = false``
- ``orderLiterals: bool = false``
- ``deduplicate: bool = false``
- ``cse: bool = false``
- ``constantOptimizer``
- ``yul: YulOptimizerDetails?``

YulOptimizerDetails
-------------------

- ``stackAllocation``
- ``optimizerSteps``

OutputSelection
---------------

