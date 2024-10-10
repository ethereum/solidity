// All objects have identical unoptimized code, but don't necessarily contain the same location comments.
// The optimized Yul will be identical only between objects that have identical debug info.
// Note that when @use-src is missing, the parser ignores location comments, so they do not become
// a part of the debug info.

/// @use-src 0:"A.sol"
object "A" {
    code {
        function load(i) -> r { r := calldataload(i) }
        /// @src 0:10:20
        sstore(load(0), load(1))
    }

    /// @use-src 0:"B.sol"
    object "B" {
        code {
            function load(i) -> r { r := calldataload(i) }
            /// @src 0:10:20
            sstore(load(0), load(1))
        }

        object "A" {
            code {
                function load(i) -> r { r := calldataload(i) }
                sstore(load(0), load(1))
            }

            object "B" {
                code {
                    function load(i) -> r { r := calldataload(i) }
                    sstore(load(0), load(1))
                }
            }

            /// @use-src 0:"C.sol"
            object "C" {
                code {
                    function load(i) -> r { r := calldataload(i) }
                    /// @src 0:10:20
                    sstore(load(0), load(1))
                }
            }
        }
    }

    /// @use-src 0:"C.sol"
    object "C" {
        code {
            function load(i) -> r { r := calldataload(i) }
            /// @src 0:10:20
            sstore(load(0), load(1))
        }
    }

    /// @use-src 0:"C.sol"
    object "D" {
        code {
            function load(i) -> r { r := calldataload(i) }
            sstore(load(0), load(1))
        }
    }

    object "E" {
        code {
            function load(i) -> r { r := calldataload(i) }
            /// @src 0:10:20
            sstore(load(0), load(1))
        }
    }
}
// ====
// optimizationPreset: full
// ----
// Assembly:
//     /* "A.sol":10:20   */
//   sstore(calldataload(0x00), calldataload(0x01))
//   stop
// stop
//
// sub_0: assembly {
//         /* "B.sol":10:20   */
//       sstore(calldataload(0x00), calldataload(0x01))
//       stop
//     stop
//
//     sub_0: assembly {
//             /* "source":621:622   */
//           0x01
//             /* "source":608:623   */
//           calldataload
//             /* "source":604:605   */
//           0x00
//             /* "source":591:606   */
//           calldataload
//             /* "source":584:624   */
//           sstore
//             /* "source":544:656   */
//           stop
//         stop
//
//         sub_0: assembly {
//                 /* "source":788:789   */
//               0x01
//                 /* "source":775:790   */
//               calldataload
//                 /* "source":771:772   */
//               0x00
//                 /* "source":758:773   */
//               calldataload
//                 /* "source":751:791   */
//               sstore
//                 /* "source":703:831   */
//               stop
//         }
//
//         sub_1: assembly {
//                 /* "C.sol":10:20   */
//               sstore(calldataload(0x00), calldataload(0x01))
//               stop
//         }
//     }
// }
//
// sub_1: assembly {
//         /* "C.sol":10:20   */
//       sstore(calldataload(0x00), calldataload(0x01))
//       stop
// }
//
// sub_2: assembly {
//       sstore(calldataload(0x00), calldataload(0x01))
//       stop
// }
//
// sub_3: assembly {
//         /* "source":1743:1744   */
//       0x01
//         /* "source":1730:1745   */
//       calldataload
//         /* "source":1726:1727   */
//       0x00
//         /* "source":1713:1728   */
//       calldataload
//         /* "source":1706:1746   */
//       sstore
//         /* "source":1674:1770   */
//       stop
// }
// Bytecode: 6001355f355500fe
// Opcodes: PUSH1 0x1 CALLDATALOAD PUSH0 CALLDATALOAD SSTORE STOP INVALID
// SourceMappings: 10:10::-:0;-1:-1;10:10;-1:-1;10:10;-1:-1
