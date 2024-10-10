// All objects have identical unoptimized code, but at 200 runs the creation objects will optimized
// differently from deployed objects.

/// @use-src 0:"A.sol"
object "A" {
    code {
        function load(i) -> r { r := calldataload(i) }
        let x := add(shl(255, 1), shl(127, not(0)))
        sstore(load(x), load(1))
    }

    /// @use-src 0:"B.sol"
    object "B_deployed" {
        code {
            function load(i) -> r { r := calldataload(i) }
            let x := add(shl(255, 1), shl(127, not(0)))
            sstore(load(x), load(1))
        }

        /// @use-src 0:"A.sol"
        object "A" {
            code {
                function load(i) -> r { r := calldataload(i) }
                let x := add(shl(255, 1), shl(127, not(0)))
                sstore(load(x), load(1))
            }

            /// @use-src 0:"B.sol"
            object "B_deployed" {
                code {
                    function load(i) -> r { r := calldataload(i) }
                    let x := add(shl(255, 1), shl(127, not(0)))
                    sstore(load(x), load(1))
                }
            }

            /// @use-src 0:"C.sol"
            object "C" {
                code {
                    function load(i) -> r { r := calldataload(i) }
                    let x := add(shl(255, 1), shl(127, not(0)))
                    sstore(load(x), load(1))
                }
            }
        }
    }

    /// @use-src 0:"C.sol"
    object "C_deployed" {
        code {
            function load(i) -> r { r := calldataload(i) }
            let x := add(shl(255, 1), shl(127, not(0)))
            sstore(load(x), load(1))
        }
    }

    /// @use-src 0:"D.sol"
    object "D" {
        code {
            function load(i) -> r { r := calldataload(i) }
            let x := add(shl(255, 1), shl(127, not(0)))
            sstore(load(x), load(1))
        }
    }
}
// ====
// optimizationPreset: full
// ----
// Assembly:
//   sstore(calldataload(sub(shl(0xff, 0x01), shl(0x7f, 0x01))), calldataload(0x01))
//   stop
// stop
//
// sub_0: assembly {
//       sstore(calldataload(shl(0x7f, 0xffffffffffffffffffffffffffffffff)), calldataload(0x01))
//       stop
//     stop
//
//     sub_0: assembly {
//           sstore(calldataload(sub(shl(0xff, 0x01), shl(0x7f, 0x01))), calldataload(0x01))
//           stop
//         stop
//
//         sub_0: assembly {
//               sstore(calldataload(shl(0x7f, 0xffffffffffffffffffffffffffffffff)), calldataload(0x01))
//               stop
//         }
//
//         sub_1: assembly {
//               sstore(calldataload(sub(shl(0xff, 0x01), shl(0x7f, 0x01))), calldataload(0x01))
//               stop
//         }
//     }
// }
//
// sub_1: assembly {
//       sstore(calldataload(shl(0x7f, 0xffffffffffffffffffffffffffffffff)), calldataload(0x01))
//       stop
// }
//
// sub_2: assembly {
//       sstore(calldataload(sub(shl(0xff, 0x01), shl(0x7f, 0x01))), calldataload(0x01))
//       stop
// }
// Bytecode: 6001356001607f1b600160ff1b03355500fe
// Opcodes: PUSH1 0x1 CALLDATALOAD PUSH1 0x1 PUSH1 0x7F SHL PUSH1 0x1 PUSH1 0xFF SHL SUB CALLDATALOAD SSTORE STOP INVALID
// SourceMappings: :::-:0;;;;;;;;;;;
