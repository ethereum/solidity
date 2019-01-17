#################
Contract Metadata
#################

.. index:: metadata, contract verification

The Solidity compiler automatically generates a JSON file, the contract
metadata, that contains information about the current contract. You can use
this file to query the compiler version, the sources used, the ABI and NatSpec
documentation to more safely interact with the contract and verify its source
code.

The compiler appends a Swarm hash of the metadata file to the end of the
bytecode (for details, see below) of each contract, so that you can retrieve
the file in an authenticated way without having to resort to a centralized
data provider.

You have to publish the metadata file to Swarm (or another service) so that
others can access it. You create the file by using the ``solc --metadata``
command that generates a file called ``ContractName_meta.json``. It contains
Swarm references to the source code, so you have to upload all source files and
the metadata file.

The metadata file has the following format. The example below is presented in a
human-readable way. Properly formatted metadata should use quotes correctly,
reduce whitespace to a minimum and sort the keys of all objects to arrive at a
unique formatting. Comments are not permitted and used here only for
explanatory purposes.

.. code-block:: none

    {
      // Required: The version of the metadata format
      version: "1",
      // Required: Source code language, basically selects a "sub-version"
      // of the specification
      language: "Solidity",
      // Required: Details about the compiler, contents are specific
      // to the language.
      compiler: {
        // Required for Solidity: Version of the compiler
        version: "0.4.6+commit.2dabbdf0.Emscripten.clang",
        // Optional: Hash of the compiler binary which produced this output
        keccak256: "0x123..."
      },
      // Required: Compilation source files/source units, keys are file names
      sources:
      {
        "myFile.sol": {
          // Required: keccak256 hash of the source file
          "keccak256": "0x123...",
          // Required (unless "content" is used, see below): Sorted URL(s)
          // to the source file, protocol is more or less arbitrary, but a
          // Swarm URL is recommended
          "urls": [ "bzzr://56ab..." ]
        },
        "mortal": {
          // Required: keccak256 hash of the source file
          "keccak256": "0x234...",
          // Required (unless "url" is used): literal contents of the source file
          "content": "contract mortal is owned { function kill() { if (msg.sender == owner) selfdestruct(owner); } }"
        }
      },
      // Required: Compiler settings
      settings:
      {
        // Required for Solidity: Sorted list of remappings
        remappings: [ ":g/dir" ],
        // Optional: Optimizer settings (enabled defaults to false)
        optimizer: {
          enabled: true,
          runs: 500
        },
        // Required for Solidity: File and name of the contract or library this
        // metadata is created for.
        compilationTarget: {
          "myFile.sol": "MyContract"
        },
        // Required for Solidity: Addresses for libraries used
        libraries: {
          "MyLib": "0x123123..."
        }
      },
      // Required: Generated information about the contract.
      output:
      {
        // Required: ABI definition of the contract
        abi: [ ... ],
        // Required: NatSpec user documentation of the contract
        userdoc: [ ... ],
        // Required: NatSpec developer documentation of the contract
        devdoc: [ ... ],
      }
    }

.. warning::
  Since the bytecode of the resulting contract contains the metadata hash, any
  change to the metadata results in a change of the bytecode. This includes
  changes to a filename or path, and since the metadata includes a hash of all the
  sources used, a single whitespace change results in different metadata, and
  different bytecode.

.. note::
    Note the ABI definition above has no fixed order. It can change with compiler versions.

Encoding of the Metadata Hash in the Bytecode
=============================================

Because we might support other ways to retrieve the metadata file in the future,
the mapping ``{"bzzr0": <Swarm hash>}`` is stored
`CBOR <https://tools.ietf.org/html/rfc7049>`_-encoded. Since the beginning of that
encoding is not easy to find, its length is added in a two-byte big-endian
encoding. The current version of the Solidity compiler thus adds the following
to the end of the deployed bytecode::

    0xa1 0x65 'b' 'z' 'z' 'r' '0' 0x58 0x20 <32 bytes swarm hash> 0x00 0x29

So in order to retrieve the data, the end of the deployed bytecode can be checked
to match that pattern and use the Swarm hash to retrieve the file.

.. note::
  The compiler currently uses the "swarm version 0" hash of the metadata,
  but this might change in the future, so do not rely on this sequence
  to start with ``0xa1 0x65 'b' 'z' 'z' 'r' '0'``. We might also
  add additional data to this CBOR structure, so the
  best option is to use a proper CBOR parser.


Usage for Automatic Interface Generation and NatSpec
====================================================

The metadata is used in the following way: A component that wants to interact
with a contract (e.g. Mist or any wallet) retrieves the code of the contract, from that
the Swarm hash of a file which is then retrieved.
That file is JSON-decoded into a structure like above.

The component can then use the ABI to automatically generate a rudimentary
user interface for the contract.

Furthermore, the wallet can use the NatSpec user documentation to display a confirmation message to the user
whenever they interact with the contract, together with requesting
authorization for the transaction signature.

Additional information about Ethereum Natural Specification (NatSpec) can be found `here <https://github.com/ethereum/wiki/wiki/Ethereum-Natural-Specification-Format>`_.

Usage for Source Code Verification
==================================

In order to verify the compilation, sources can be retrieved from Swarm
via the link in the metadata file.
The compiler of the correct version (which is checked to be part of the "official" compilers)
is invoked on that input with the specified settings. The resulting
bytecode is compared to the data of the creation transaction or ``CREATE`` opcode data.
This automatically verifies the metadata since its hash is part of the bytecode.
Excess data corresponds to the constructor input data, which should be decoded
according to the interface and presented to the user.
