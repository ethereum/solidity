// All objects have unoptimized code that's identical except for subobject and data object naming.
// The optimized Yul code does not depend on the content of subobjects and data objects,
// only on their names. EVM assembly, on the other hand, is not affected by names - it uses indices
// for subobjects and hashes for data objects.

/// @use-src 0:"A.sol"
object "A" {
    code {
        function load(i) -> r { r := calldataload(i) }
        sstore(load(0), load(dataoffset("B")))
    }

    /// @use-src 0:"B.sol"
    object "B" {
        code {
            function load(i) -> r { r := calldataload(i) }
            sstore(load(0), load(dataoffset("A")))
        }

        /// @use-src 0:"A.sol"
        object "A" {
            code {
                function load(i) -> r { r := calldataload(i) }
                sstore(load(0), load(dataoffset("B")))
            }

            /// @use-src 0:"B.sol"
            object "B" {
                code {
                    function load(i) -> r { r := calldataload(i) }
                    sstore(load(0), load(dataoffset("A")))
                }

                /// @use-src 0:"A.sol"
                object "A" {
                    code {
                        function load(i) -> r { r := calldataload(i) }
                        sstore(load(0), load(dataoffset("B")))
                    }

                    data "B" "0xbb"
                }
            }

            /// @use-src 0:"C.sol"
            object "C" {
                code {
                    function load(i) -> r { r := calldataload(i) }
                    sstore(load(0), load(dataoffset("A")))
                }

                data "A" "0xaa"
            }

            /// @use-src 0:"D.sol"
            object "D" {
                code {
                    function load(i) -> r { r := calldataload(i) }
                    sstore(load(0), load(dataoffset("A")))
                }

                data "A" "0xaaaaaa"
            }
        }
    }

    /// @use-src 0:"C.sol"
    object "C" {
        code {
            function load(i) -> r { r := calldataload(i) }
            sstore(load(0), load(dataoffset("A")))
        }

        data "A" "0xaaaa"
    }

    /// @use-src 0:"D.sol"
    object "D" {
        code {
            function load(i) -> r { r := calldataload(i) }
            sstore(load(0), load(dataoffset("B")))
        }

        data "B" "0xbbbb"
    }

    /// @use-src 0:"E.sol"
    object "E" {
        code {
            function load(i) -> r { r := calldataload(i) }
            sstore(load(0), load(dataoffset("B")))
        }

        data "B" "0xbbbb"
    }
}
// ====
// optimizationPreset: full
// ----
// Assembly:
//   sstore(calldataload(0x00), calldataload(dataOffset(sub_0)))
//   stop
// stop
//
// sub_0: assembly {
//       sstore(calldataload(0x00), calldataload(dataOffset(sub_0)))
//       stop
//     stop
//
//     sub_0: assembly {
//           sstore(calldataload(0x00), calldataload(dataOffset(sub_0)))
//           stop
//         stop
//
//         sub_0: assembly {
//               sstore(calldataload(0x00), calldataload(dataOffset(sub_0)))
//               stop
//             stop
//
//             sub_0: assembly {
//                   sstore(calldataload(0x00), calldataload(data_736ddcdd19b41ff3aa09bd89628fc69562c2e39bdb07c1971217d2e374ce6e27))
//                   stop
//                 stop
//                 data_736ddcdd19b41ff3aa09bd89628fc69562c2e39bdb07c1971217d2e374ce6e27 30786262
//             }
//         }
//
//         sub_1: assembly {
//               sstore(calldataload(0x00), calldataload(data_89b1fd0f9a40d0a598af5f997daf99fc3d5b98ef4eb429e81755ae0ee49e194e))
//               stop
//             stop
//             data_89b1fd0f9a40d0a598af5f997daf99fc3d5b98ef4eb429e81755ae0ee49e194e 30786161
//         }
//
//         sub_2: assembly {
//               sstore(calldataload(0x00), calldataload(data_d8bbdaa6a33092aa88c90c124bdeea5160229ede56dd7d897c2413f9ba7e4f85))
//               stop
//             stop
//             data_d8bbdaa6a33092aa88c90c124bdeea5160229ede56dd7d897c2413f9ba7e4f85 3078616161616161
//         }
//     }
// }
//
// sub_1: assembly {
//       sstore(calldataload(0x00), calldataload(data_79f4f313d7d500fd317e5283225bdfe6ea44da3fb73f1fdbf19929f3164bfc1a))
//       stop
//     stop
//     data_79f4f313d7d500fd317e5283225bdfe6ea44da3fb73f1fdbf19929f3164bfc1a 307861616161
// }
//
// sub_2: assembly {
//       sstore(calldataload(0x00), calldataload(data_257dd53638d790f0311f866044a2856def338f9a4d22fce0bdeed5b2d53851a9))
//       stop
//     stop
//     data_257dd53638d790f0311f866044a2856def338f9a4d22fce0bdeed5b2d53851a9 307862626262
// }
//
// sub_3: assembly {
//       sstore(calldataload(0x00), calldataload(data_257dd53638d790f0311f866044a2856def338f9a4d22fce0bdeed5b2d53851a9))
//       stop
//     stop
//     data_257dd53638d790f0311f866044a2856def338f9a4d22fce0bdeed5b2d53851a9 307862626262
// }
// Bytecode: 6008355f355500fe6008355f355500fe6008355f355500fe6008355f355500fe6008355f355500fe30786262
// Opcodes: PUSH1 0x8 CALLDATALOAD PUSH0 CALLDATALOAD SSTORE STOP INVALID PUSH1 0x8 CALLDATALOAD PUSH0 CALLDATALOAD SSTORE STOP INVALID PUSH1 0x8 CALLDATALOAD PUSH0 CALLDATALOAD SSTORE STOP INVALID PUSH1 0x8 CALLDATALOAD PUSH0 CALLDATALOAD SSTORE STOP INVALID PUSH1 0x8 CALLDATALOAD PUSH0 CALLDATALOAD SSTORE STOP INVALID ADDRESS PUSH25 0x62620000000000000000000000000000000000000000000000
// SourceMappings: :::-:0;;;;;
