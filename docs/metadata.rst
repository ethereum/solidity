.. _metadata:

#################
Contract Metadata
#################

.. index:: metadata, contract verification

The Solidity compiler automatically generates a JSON file.
The file contains two kinds of information about the compiled contract:

- How to interact with the contract: ABI, and NatSpec documentation.
- How to reproduce the compilation and verify a deployed contract:
  compiler version, compiler settings, and source files used.

The compiler appends by default the IPFS hash of the metadata file to the end
of the runtime bytecode (not necessarily the creation bytecode) of each contract,
so that, if published, you can retrieve the file in an authenticated way without
having to resort to a centralized data provider. The other available options are
the Swarm hash and not appending the metadata hash to the bytecode. These can be
configured via the :ref:`Standard JSON Interface<compiler-api>`.

You have to publish the metadata file to IPFS, Swarm, or another service so
that others can access it. You create the file by using the ``solc --metadata``
command together with the ``--output-dir`` parameter. Without the parameter,
the metadata will be written to standard output.
The metadata contains IPFS and Swarm references to the source code, so you have to
upload all source files in addition to the metadata file. For IPFS, the hash contained
in the CID returned by ``ipfs add`` (not the direct sha2-256 hash of the file)
shall match with the one contained in the bytecode.

The metadata file has the following format. The example below is presented in a
human-readable way. Properly formatted metadata should use quotes correctly,
reduce whitespace to a minimum, and sort the keys of all objects in alphabetical order
to arrive at a canonical formatting. Comments are not permitted and are used here only for
explanatory purposes.

.. code-block:: javascript

    {
      // Required: Details about the compiler, contents are specific
      // to the language.
      "compiler": {
        // Optional: Hash of the compiler binary which produced this output
        "keccak256": "0x123...",
        // Required for Solidity: Version of the compiler
        "version": "0.8.2+commit.661d1103"
      },
      // Required: Source code language, basically selects a "sub-version"
      // of the specification
      "language": "Solidity",
      // Required: Generated information about the contract.
      "output": {
        // Required: ABI definition of the contract. See "Contract ABI Specification"
        "abi": [/* ... */],
        // Required: NatSpec developer documentation of the contract. See https://docs.soliditylang.org/en/latest/natspec-format.html for details.
        "devdoc": {
          // Contents of the @author NatSpec field of the contract
          "author": "John Doe",
          // Contents of the @dev NatSpec field of the contract
          "details": "Interface of the ERC20 standard as defined in the EIP. See https://eips.ethereum.org/EIPS/eip-20 for details",
          "errors": {
            "MintToZeroAddress()" : {
              "details": "Cannot mint to zero address"
            }
          },
          "events": {
            "Transfer(address,address,uint256)": {
              "details": "Emitted when `value` tokens are moved from one account (`from`) toanother (`to`).",
              "params": {
                "from": "The sender address",
                "to": "The receiver address",
                "value": "The token amount"
              }
            }
          },
          "kind": "dev",
          "methods": {
            "transfer(address,uint256)": {
              // Contents of the @dev NatSpec field of the method
              "details": "Returns a boolean value indicating whether the operation succeeded. Must be called by the token holder address",
              // Contents of the @param NatSpec fields of the method
              "params": {
                "_value": "The amount tokens to be transferred",
                "_to": "The receiver address"
              },
              // Contents of the @return NatSpec field.
              "returns": {
                // Return var name (here "success") if exists. "_0" as key if return var is unnamed
                "success": "a boolean value indicating whether the operation succeeded"
              }
            }
          },
          "stateVariables": {
            "owner": {
              // Contents of the @dev NatSpec field of the state variable
              "details": "Must be set during contract creation. Can then only be changed by the owner"
            }
          },
          // Contents of the @title NatSpec field of the contract
          "title": "MyERC20: an example ERC20",
          "version": 1 // NatSpec version
        },
        // Required: NatSpec user documentation of the contract. See "NatSpec Format"
        "userdoc": {
          "errors": {
            "ApprovalCallerNotOwnerNorApproved()": [
              {
                "notice": "The caller must own the token or be an approved operator."
              }
            ]
          },
          "events": {
            "Transfer(address,address,uint256)": {
              "notice": "`_value` tokens have been moved from `from` to `to`"
            }
          },
          "kind": "user",
          "methods": {
            "transfer(address,uint256)": {
              "notice": "Transfers `_value` tokens to address `_to`"
            }
          },
          "version": 1 // NatSpec version
        }
      },
      // Required: Compiler settings. Reflects the settings in the JSON input during compilation.
      // Check the documentation of standard JSON input's "settings" field
      "settings": {
        // Required for Solidity: File path and the name of the contract or library this
        // metadata is created for.
        "compilationTarget": {
          "myDirectory/myFile.sol": "MyContract"
        },
        // Required for Solidity.
        "evmVersion": "london",
        // Required for Solidity: Addresses for libraries used.
        "libraries": {
          "MyLib": "0x123123..."
        },
        "metadata": {
          // Reflects the setting used in the input json, defaults to "true"
          "appendCBOR": true,
          // Reflects the setting used in the input json, defaults to "ipfs"
          "bytecodeHash": "ipfs",
          // Reflects the setting used in the input json, defaults to "false"
          "useLiteralContent": true
        },
        // Optional: Optimizer settings. The fields "enabled" and "runs" are deprecated
        // and are only given for backward-compatibility.
        "optimizer": {
          "details": {
            "constantOptimizer": false,
            "cse": false,
            "deduplicate": false,
            // inliner defaults to "false"
            "inliner": false,
            // jumpdestRemover defaults to "true"
            "jumpdestRemover": true,
            "orderLiterals": false,
            // peephole defaults to "true"
            "peephole": true,
            "yul": true,
            // Optional: Only present if "yul" is "true"
            "yulDetails": {
              "optimizerSteps": "dhfoDgvulfnTUtnIf...",
              "stackAllocation": false
            }
          },
          "enabled": true,
          "runs": 500
        },
        // Required for Solidity: Sorted list of import remappings.
        "remappings": [ ":g=/dir" ]
      },
      // Required: Compilation source files/source units, keys are file paths
      "sources": {
        "destructible": {
          // Required (unless "url" is used): literal contents of the source file
          "content": "contract destructible is owned { function destroy() { if (msg.sender == owner) selfdestruct(owner); } }",
          // Required: keccak256 hash of the source file
          "keccak256": "0x234..."
        },
        "myDirectory/myFile.sol": {
          // Required: keccak256 hash of the source file
          "keccak256": "0x123...",
          // Optional: SPDX license identifier as given in the source file
          "license": "MIT",
          // Required (unless "content" is used, see above): Sorted URL(s)
          // to the source file, protocol is more or less arbitrary, but an
          // IPFS URL is recommended
          "urls": [ "bzz-raw://7d7a...", "dweb:/ipfs/QmN..." ]
        }
      },
      // Required: The version of the metadata format
      "version": 1
    }

.. warning::
  Since the bytecode of the resulting contract contains the metadata hash by default, any
  change to the metadata might result in a change of the bytecode. This includes
  changes to a filename or path, and since the metadata includes a hash of all the
  sources used, a single whitespace change results in different metadata, and
  different bytecode.

.. note::
    The ABI definition above has no fixed order. It can change with compiler versions.
    Starting from Solidity version 0.5.12, though, the array maintains a certain
    order.

.. _encoding-of-the-metadata-hash-in-the-bytecode:

Encoding of the Metadata Hash in the Bytecode
=============================================

The compiler currently by default appends the
`IPFS hash (in CID v0) <https://docs.ipfs.tech/concepts/content-addressing/#version-0-v0>`_
of the canonical metadata file and the compiler version to the end of the bytecode.
Optionally, a Swarm hash instead of the IPFS, or an experimental flag is used.
Below are all the possible fields:

.. code-block:: javascript

    {
      "ipfs": "<metadata hash>",
      // If "bytecodeHash" was "bzzr1" in compiler settings not "ipfs" but "bzzr1"
      "bzzr1": "<metadata hash>",
      // Previous versions were using "bzzr0" instead of "bzzr1"
      "bzzr0": "<metadata hash>",
      // If any experimental features that affect code generation are used
      "experimental": true,
      "solc": "<compiler version>"
    }

Because we might support other ways to retrieve the
metadata file in the future, this information is stored
`CBOR <https://tools.ietf.org/html/rfc7049>`_-encoded. The last two bytes in the bytecode
indicate the length of the CBOR encoded information. By looking at this length, the
relevant part of the bytecode can be decoded with a CBOR decoder.

Check the `Metadata Playground <https://playground.sourcify.dev/>`_ to see it in action.

Whereas release builds of solc use a 3 byte encoding of the version as shown
above (one byte each for major, minor and patch version number), pre-release builds
will instead use a complete version string including commit hash and build date.

The commandline flag ``--no-cbor-metadata`` can be used to skip metadata
from getting appended at the end of the deployed bytecode. Equivalently, the
boolean field ``settings.metadata.appendCBOR`` in Standard JSON input can be set to false.

.. note::
  The CBOR mapping can also contain other keys, so it is better to fully
  decode the data by looking at the end of the bytecode for the CBOR length,
  and to use a proper CBOR parser. Do not rely on it starting with ``0xa264``
  or ``0xa2 0x64 'i' 'p' 'f' 's'``.

Usage for Automatic Interface Generation and NatSpec
====================================================

The metadata is used in the following way: A component that wants to interact
with a contract (e.g. a wallet) retrieves the code of the contract.
It decodes the CBOR encoded section containing the IPFS/Swarm hash of the
metadata file. With that hash, the metadata file is retrieved. That file
is JSON-decoded into a structure like above.

The component can then use the ABI to automatically generate a rudimentary
user interface for the contract.

Furthermore, the wallet can use the NatSpec user documentation to display a
human-readable confirmation message to the user whenever they interact with
the contract, together with requesting authorization for the transaction signature.

For additional information, read :doc:`Ethereum Natural Language Specification (NatSpec) format <natspec-format>`.

Usage for Source Code Verification
==================================

If pinned/published, it is possible to retrieve the metadata of the contract from IPFS/Swarm.
The metadata file also contains the URLs or the IPFS hashes of the source files, as well as
the compilation settings, i.e. everything needed to reproduce a compilation.

With this information it is then possible to verify the source code of a contract by
reproducing the compilation, and comparing the bytecode from the compilation with
the bytecode of the deployed contract.

This automatically verifies the metadata since its hash is part of the bytecode, as well
as the source codes, because their hashes are part of the metadata. Any change in the files
or settings would result in a different metadata hash. The metadata here serves
as a fingerprint of the whole compilation.

`Sourcify <https://sourcify.dev>`_ makes use of this feature for "full/perfect verification",
as well as pinning the files publicly on IPFS to be accessed with the metadata hash.
