.. index:: ! commandline compiler, compiler;commandline, ! solc, ! linker

.. _commandline-compiler:

******************************
Using the Commandline Compiler
******************************

One of the build targets of the Solidity repository is ``solc``, the solidity commandline compiler.
Using ``solc --help`` provides you with an explanation of all options. The compiler can produce various outputs, ranging from simple binaries and assembly over an abstract syntax tree (parse tree) to estimations of gas usage.
If you only want to compile a single file, you run it as ``solc --bin sourceFile.sol`` and it will print the binary. Before you deploy your contract, activate the optimizer while compiling using ``solc --optimize --bin sourceFile.sol``. If you want to get some of the more advanced output variants of ``solc``, it is probably better to tell it to output everything to separate files using ``solc -o outputDirectory --bin --ast --asm sourceFile.sol``.

The commandline compiler will automatically read imported files from the filesystem, but
it is also possible to provide path redirects using ``prefix=path`` in the following way:

::

    solc github.com/ethereum/dapp-bin/=/usr/local/lib/dapp-bin/ =/usr/local/lib/fallback file.sol

This essentially instructs the compiler to search for anything starting with
``github.com/ethereum/dapp-bin/`` under ``/usr/local/lib/dapp-bin`` and if it does not
find the file there, it will look at ``/usr/local/lib/fallback`` (the empty prefix
always matches). ``solc`` will not read files from the filesystem that lie outside of
the remapping targets and outside of the directories where explicitly specified source
files reside, so things like ``import "/etc/passwd";`` only work if you add ``=/`` as a remapping.

If there are multiple matches due to remappings, the one with the longest common prefix is selected.

If your contracts use :ref:`libraries <libraries>`, you will notice that the bytecode contains substrings of the form ``__LibraryName______``. You can use ``solc`` as a linker meaning that it will insert the library addresses for you at those points:

Either add ``--libraries "Math:0x12345678901234567890 Heap:0xabcdef0123456"`` to your command to provide an address for each library or store the string in a file (one library per line) and run ``solc`` using ``--libraries fileName``.

If ``solc`` is called with the option ``--link``, all input files are interpreted to be unlinked binaries (hex-encoded) in the ``__LibraryName____``-format given above and are linked in-place (if the input is read from stdin, it is written to stdout). All options except ``--libraries`` are ignored (including ``-o``) in this case.


*****************************************
Standardized Input and Output Description
*****************************************

The compiler API expects a JSON formatted input and outputs the compilations result in a JSON formatted output.

Comments are of course not permitted and used here only for explanatory purposes.

Input Description
-----------------

QUESTION: How to specific file-reading callback? - probably not as part of json input

The input description is language-specific and could change with each compiler version, but it
should be backwards compatible if possible.

.. code-block:: none

    {
      // Required
      sources:
      {
        // The keys here are the "global" names of the source files,
        // imports can use other files via remappings (see below).
        "myFile.sol":
        {
          // Optional: keccak256 hash of the source file
          "keccak256": "0x123...",
          // Required (unless "content" is used, see below): URL(s) to the source file.
          // URL(s) should be imported in this order and the result checked against the
          // keccak256 hash (if available). If the hash doesn't match or none of the
          // URL(s) result in success, an error should be raised.
          "urls":
          [
            "bzzr://56ab...",
            "ipfs://Qma...",
            "file:///tmp/path/to/file.sol"
          ]
        },
        "mortal":
        {
          // Optional: keccak256 hash of the source file
          "keccak256": "0x234...",
          // Required (unless "urls" is used): literal contents of the source file
          "content": "contract mortal is owned { function kill() { if (msg.sender == owner) selfdestruct(owner); } }"
        }
      },
      // Optional
      settings:
      {
        // Optional: Sorted list of remappings
        remappings: [ ":g/dir" ],
        // Optional: Optimizer settings (enabled defaults to false)
        optimizer: {
          enabled: true,
          runs: 500
        },
        // If given, only compiles the specified contracts.
        compilationTarget: {
          "myFile.sol": "MyContract"
        },
        // Addresses of the libraries. If not all libraries are given here, it can result in unlinked objects whose output data is different.
        libraries: {
          // The top level key is the the name of the source file where the library is used.
          // If remappings are used, this source file should match the global path after remappings were applied.
          // If this key is an empty string, that refers to a global level.
          "myFile.sol": {
            "MyLib": "0x123123..."
          }
        }
        // The following can be used to restrict the fields the compiler will output.
        // (axic)
        outputSelection: [
            "abi", "evm.assembly", "evm.bytecode", ..., "why3", "ewasm.wasm"
        ]
        outputSelection: {
        abi,asm,ast,bin,bin-runtime,clone-bin,devdoc,interface,opcodes,srcmap,srcmap-runtime,userdoc

 --ast                 AST of all source files.
  --ast-json            AST of all source files in JSON format.
  --asm                 EVM assembly of the contracts.
  --asm-json            EVM assembly of the contracts in JSON format.
  --opcodes             Opcodes of the contracts.
  --bin                 Binary of the contracts in hex.
  --bin-runtime         Binary of the runtime part of the contracts in hex.
  --clone-bin           Binary of the clone contracts in hex.
  --abi                 ABI specification of the contracts.
  --interface           Solidity interface of the contracts.
  --hashes              Function signature hashes of the contracts.
  --userdoc             Natspec user documentation of all contracts.
  --devdoc              Natspec developer documentation of all contracts.
  --formal              Translated source suitable for formal analysis.

          // to be defined
        }
      }
    }


Regular Output
--------------

.. code-block:: none

    {
      errors: ["error1", "error2"], // we might structure them
      errors: [
          {
              // (axic)
              file: "sourceFile.sol", // optional?
              contract: "contractName", // optional
              line: 100, // optional - currently, we always have a byte range in the source file
              // Errors/warnings originate in several components, most of them are not
              // backend-specific. Currently, why3 errors are part of the why3 output.
              // I think it is better to put code-generator-specific errors into the code-generator output
              // area, and warnings and errors that are code-generator-agnostic into this general area,
              // so that it is easier to determine whether some source code is invalid or only
              // triggers errors/warnings in some backend that might only implement some part of solidity.
              type: "evm" or "why3" or "ewasm" // maybe a better field name would be needed
              severity: "warning" or "error" // mandatory
              message: "Invalid keyword" // mandatory
          }
      ]
      // This contains all the compiled outputs. It can be limited/filtered by the compilationTarget setting.
      contracts: {
        "sourceFile.sol:ContractName": {
          // The Ethereum Contract ABI. If empty, it is represented as an empty array.
          // See https://github.com/ethereum/wiki/wiki/Ethereum-Contract-ABI
          abi: [],
          evm: {
              assembly:
              bytecode:
              runtimeBytecode:
              opcodes:
              annotatedOpcodes: // (axic) see https://github.com/ethereum/solidity/issues/1178
              gasEstimates:
              sourceMap:
              runtimeSourceMap:
              // If given, this is an unlinked object (cannot be filtered out explicitly, might be
              // filtered if both bytecode, runtimeBytecode, opcodes and others are filtered out)
              linkReferences: {
                "sourceFile.sol:Library1": [1, 200, 80] // byte offsets into bytecode. Linking replaces the 20 bytes there.
              }
              // the same for runtimeBytecode - I'm not sure it is a good idea to allow to link libraries differently for the runtime bytecode.
              // furthermore, runtime bytecode is always a substring of the bytecode anyway.
              runtimeLinkReferences: {
              }
          },
          functionHashes:
          metadata: // see the Metadata Output documentation
          ewasm: {
              wast: // S-expression format
              wasm: //
          },
          userdoc: // Obsolete
          devdoc: // Obsolete
          natspec: // Combined dev+userdoc
        }
      },
      formal: {
        "why3": "..."
      },
      sourceList: ["source1.sol", "source2.sol"], // this is important for source references both in the ast as well as in the srcmap in the contract
      sources: {
        "source1.sol": {
          "AST": { ... }
        }
      }
    }
