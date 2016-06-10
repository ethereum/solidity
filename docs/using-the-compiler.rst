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
there is a standardized for describing the relations between those files.
Furthermore, the compiler can generate a json file while compiling that includes
the source, natspec comments and other metadata whose hash is included in the
actual bytecode.

There is some overlap between the input description and the metadata output
and due to the fact that some fields are optional, the metadata can be used as
input to the compiler. In order to verify the metadata, you actually take it,
re-run the compiler on the metadata and check that it again produces the same
metadata.

If the compiler is invoked in a different way, not using the input
description (for example by using a file content retrieval callback),
the compiler can still generate the metadata alongside the bytecode of each
contract.

The metadata standard is versioned. Future versions are only required to provide the "version" field,
the two keys inside the "compiler" field. The field compiler.keccak should be the keccak hash
of a binary of the compiler with the given version.

The example below is presented in a human-readable way. Properly formatted metadata
should use quotes correctly, reduce whitespace to a minimum and sort the keys of all objects
to arrive at a unique formatting.

Comments are of course not permitted and used here only for explanatory purposes.

Input Description
-----------------

The input description could change with each compiler version, but it
should be backwards compatible if possible.

    {
      sources:
      {
        "abc": "contract b{}",
        "def": {keccak: "0x123..."}, // source has to be retrieved by its hash
        "dir/file.sol": "contract a {}"
      },
      settings:
      {
        remappings: [":g/dir"],
        optimizer: {enabled: true, runs: 500},
        compilationTarget: "myFile.sol:MyContract", // Can also be an array
        // To be backwards compatible, use the full name of the contract in the output
        // only if there are name clashes.
        // If the full name was given as "compilationTargets", use the full name.
        libraries: {
          "def:MyLib": "0x123123..."
        },
        // The following can be used to restrict the fields the compiler will output.
        outputSelection: {
          // to be defined
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
      compiler: {
        version: "soljson-2313-2016-12-12",
        keccak: "0x123..."
      },
      sources:
      {
        "abc": "contract b{}",
        "def": {keccak: "0x123..."}, // source has to be retrieved by its hash
        "dir/file.sol": "contract a {}"
      },
      settings:
      {
        remappings: [":g/dir"],
        optimizer: {enabled: true, runs: 500},
        compilationTarget: "myFile.sol:MyContract",
        // To be backwards compatible, use the full name of the contract in the output
        // only if there are name clashes.
        // If the full name was given as "compilationTargets", use the full name.
        libraries: {
          "def:MyLib": "0x123123..."
        }
      },
      output:
      {
        abi: [ /* abi definition */ ],
        userDocumentation: [ /* user documentation comments */ ],
        developerDocumentation: [ /* developer documentation comments */ ],
        natspec: [ /* natspec comments */ ]
      }
    }
