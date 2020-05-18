object "A" {
  code {
    pop(dataoffset("A"))
    pop(dataoffset("B"))
    pop(dataoffset("B.C"))
    pop(dataoffset("B.C.D"))
  }

  object "B" {
    code {
      pop(dataoffset("C"))
      pop(dataoffset("C.D"))
    }
    object "C" {
      code {
        pop(dataoffset("D"))
      }
      object "D" {
        code {
          invalid()
        }
      }
    }
  }
}
// ----
// Assembly:
//     /* "source":26:46   */
//   pop(0x00)
//     /* "source":51:71   */
//   pop(dataOffset(sub_0))
//     /* "source":76:98   */
//   pop(dataOffset(sub_1))
//     /* "source":103:127   */
//   pop(dataOffset(sub_2))
// stop
//
// sub_0: assembly {
//         /* "source":165:185   */
//       pop(dataOffset(sub_0))
//         /* "source":192:214   */
//       pop(dataOffset(sub_1))
//     stop
//
//     sub_0: assembly {
//             /* "source":259:279   */
//           pop(dataOffset(sub_0))
//         stop
//
//         sub_0: assembly {
//                 /* "source":332:341   */
//               invalid
//         }
//     }
//
//     sub_1: assembly {
//             /* "source":332:341   */
//           invalid
//     }
// }
//
// sub_1: assembly {
//         /* "source":259:279   */
//       pop(dataOffset(sub_0))
//     stop
//
//     sub_0: assembly {
//             /* "source":332:341   */
//           invalid
//     }
// }
//
// sub_2: assembly {
//         /* "source":332:341   */
//       invalid
// }
// Bytecode: 600050600d50601a50601f50fe600750600c50fe600450fefefe600450fefefe
// Opcodes: PUSH1 0x0 POP PUSH1 0xD POP PUSH1 0x1A POP PUSH1 0x1F POP INVALID PUSH1 0x7 POP PUSH1 0xC POP INVALID PUSH1 0x4 POP INVALID INVALID INVALID PUSH1 0x4 POP INVALID INVALID INVALID
// SourceMappings: 26:20:0:-:0;;51;;76:22;;103:24;
