// All objects have identical unoptimized code, but don't necessarily contain the same location comments.
// After optimizations we should end up with identical code for all of them, because location
// comments will be ignored due to missing @use-src annotations and won't end up in the AST.

object "A" {
    code {
        function load(i) -> r { r := calldataload(i) }
        /// @src 0:10:20
        sstore(load(0), load(1))
    }

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

            object "C" {
                code {
                    function load(i) -> r { r := calldataload(i) }
                    /// @src 0:10:20
                    sstore(load(0), load(1))
                }
            }
        }
    }

    object "C" {
        code {
            function load(i) -> r { r := calldataload(i) }
            /// @src 0:10:20
            sstore(load(0), load(1))
        }
    }

    object "D" {
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
//     /* "source":83:84   */
//   0x01
//     /* "source":70:85   */
//   calldataload
//     /* "source":66:67   */
//   0x00
//     /* "source":53:68   */
//   calldataload
//     /* "source":46:86   */
//   sstore
//     /* "source":22:102   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":202:203   */
//       0x01
//         /* "source":189:204   */
//       calldataload
//         /* "source":185:186   */
//       0x00
//         /* "source":172:187   */
//       calldataload
//         /* "source":165:205   */
//       sstore
//         /* "source":133:229   */
//       stop
//     stop
//
//     sub_0: assembly {
//             /* "source":345:346   */
//           0x01
//             /* "source":332:347   */
//           calldataload
//             /* "source":328:329   */
//           0x00
//             /* "source":315:330   */
//           calldataload
//             /* "source":308:348   */
//           sstore
//             /* "source":268:380   */
//           stop
//         stop
//
//         sub_0: assembly {
//                 /* "source":512:513   */
//               0x01
//                 /* "source":499:514   */
//               calldataload
//                 /* "source":495:496   */
//               0x00
//                 /* "source":482:497   */
//               calldataload
//                 /* "source":475:515   */
//               sstore
//                 /* "source":427:555   */
//               stop
//         }
//
//         sub_1: assembly {
//                 /* "source":701:702   */
//               0x01
//                 /* "source":688:703   */
//               calldataload
//                 /* "source":684:685   */
//               0x00
//                 /* "source":671:686   */
//               calldataload
//                 /* "source":664:704   */
//               sstore
//                 /* "source":616:744   */
//               stop
//         }
//     }
// }
//
// sub_1: assembly {
//         /* "source":874:875   */
//       0x01
//         /* "source":861:876   */
//       calldataload
//         /* "source":857:858   */
//       0x00
//         /* "source":844:859   */
//       calldataload
//         /* "source":837:877   */
//       sstore
//         /* "source":805:901   */
//       stop
// }
//
// sub_2: assembly {
//         /* "source":1007:1008   */
//       0x01
//         /* "source":994:1009   */
//       calldataload
//         /* "source":990:991   */
//       0x00
//         /* "source":977:992   */
//       calldataload
//         /* "source":970:1010   */
//       sstore
//         /* "source":938:1034   */
//       stop
// }
// Bytecode: 6001355f355500fe
// Opcodes: PUSH1 0x1 CALLDATALOAD PUSH0 CALLDATALOAD SSTORE STOP INVALID
// SourceMappings: 83:1:0:-:0;70:15;66:1;53:15;46:40;22:80
