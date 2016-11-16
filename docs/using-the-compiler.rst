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


**************************************************
Standardized Input Description and Metadata Output
**************************************************

In order to ease source code verification of complex contracts that are spread across several files,
there is a standardized way for describing the relations between those files.
Furthermore, the compiler can generate a json file while compiling that includes
the (hash of the) source, natspec comments and other metadata whose hash is included in the
actual bytecode. Specifically, the creation data for a contract has to begin with
`push32 <metadata hash> pop`.

The metadata standard is versioned. Future versions are only required to provide the "version" field,
the "language" field and the two keys inside the "compiler" field.
The field compiler.keccak should be the keccak hash of a binary of the compiler with the given version.

The example below is presented in a human-readable way. Properly formatted metadata
should use quotes correctly, reduce whitespace to a minimum and sort the keys of all objects
to arrive at a unique formatting.

Comments are of course not permitted and used here only for explanatory purposes.

Input Description
-----------------

QUESTION: How to specific file-reading callback? - probably not as part of json input

The input description is language-specific and could change with each compiler version, but it
should be backwards compatible if possible.

    {
      sources:
      {
        // the keys here are the "global" names of the source files, imports can use other files via remappings (see below)
        "abc": "contract b{}", // specify source directly
        // (axic) I think 'keccak' on its on is not enough. I would go perhaps with swarm: "0x12.." and ipfs: "Qma..." for simplicity
        // (chriseth) Where the content is stored is a second component, but yes, we could give an indication there.
        "def": {keccak: "0x123..."}, // source has to be retrieved by its hash
        "ghi": {file: "/tmp/path/to/file.sol"}, // file on filesystem
        // (axic) I'm inclined to think the source _must_ be provided in the JSON,

        "dir/file.sol": "contract a {}"
      },
      settings:
      {
        remappings: [":g/dir"], // just as it used to be
        // (axic) what is remapping doing exactly?
        optimizer: {enabled: true, runs: 500},
        // if given, only compiles this contract, can also be an array. If only a contract name is given, tries to find it if unique.
        compilationTarget: "myFile.sol:MyContract",
        // addresses of the libraries. If not all libraries are given here, it can result in unlinked objects whose output data is different
        libraries: {
          "def:MyLib": "0x123123..."
        },
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
          metadata: // see below
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

Metadata Output
---------------

Note that the actual bytecode is not part of the metadata because the hash
of the metadata structure will be included in the bytecode itself.

This requires the compiler to be able to compute the hash of its own binary,
which requires it to be statically linked. The hash of the binary is not
too important. It is much more important to have the commit hash because
that can be used to query a location of the binary (and whether the version is
"official") at a registry contract.

    {
      version: "1",
      language: "Solidity",
      compiler: {
        commit: "55db20e32c97098d13230ab7500758e8e3b31d64",
        version: "soljson-2313-2016-12-12",
        keccak: "0x123..."
      },
      // This is a subset of the regular compiler input
      sources:
      {
        "abc": {keccak: "0x456..."}, // here, sources are always given by hash
        "def": {keccak: "0x123..."},
        "dir/file.sol": {keccax: "0xabc..."},
        "xkcd": {swarm: "0x456..."}
      },
      // This is a subset of the regular compiler input
      settings:
      {
        remappings: [":g/dir"],
        optimizer: {enabled: true, runs: 500},
        compilationTarget: "myFile.sol:MyContract",
        libraries: {
          "def:MyLib": "0x123123..."
        }
      },
      // This is a subset of the regular compiler output
      output:
      {
        abi: [ /* abi definition */ ],
        userdoc: [],
        devdoc: [],
        natspec: [ /* user documentation comments */ ]
      }
    }

This is used in the following way: A component that wants to interact
with a contract (e.g. mist) retrieves the creation transaction of the contract
and from that the first 33 bytes. If the first byte decodes into a PUSH32
instruction, the other 32 bytes are interpreted as the keccak-hash of
a file which is retrieved via a content-addressable storage like swarm.
That file is JSON-decoded into a structure like above. Sources are
retrieved in the same way and combined with the structure into a proper
compiler input description, which selects only the bytecode as output.

The compiler of the correct version (which is checked to be part of the "official" compilers)
is invoked on that input. The resulting
bytecode is compared (excess bytecode in the creation transaction
is constructor input data) which automatically verifies the metadata since
its hash is part of the bytecode. The constructor input data is decoded
according to the interface and presented to the user.
