object "A" {
  code {
    pop(dataoffset("A"))
    pop(datasize("A"))
    pop(dataoffset("B"))
    pop(datasize("B"))
    pop(dataoffset("B.C"))
    pop(datasize("B.C"))
    pop(dataoffset("B.E"))
    pop(datasize("B.E"))
    pop(dataoffset("B.C.D"))
    pop(datasize("B.C.D"))
  }

  object "B" {
    code {
      pop(dataoffset("C"))
      pop(datasize("C"))
      pop(dataoffset("E"))
      pop(datasize("E"))
      pop(dataoffset("C.D"))
      pop(datasize("C.D"))
    }
    object "C" {
      code {
        pop(dataoffset("D"))
        pop(datasize("D"))
      }
      object "D" {
        code {
          invalid()
        }
      }
    }
    object "E" {
      code {
        invalid()
      }
    }
  }
}
// ----
// Assembly:
//     /* "source":26:46   */
//   pop(0x00)
//     /* "source":51:69   */
//   pop(bytecodeSize)
//     /* "source":74:94   */
//   pop(dataOffset(sub_0))
//     /* "source":99:117   */
//   pop(dataSize(sub_0))
//     /* "source":122:144   */
//   pop(dataOffset(sub_0.sub_0))
//     /* "source":149:169   */
//   pop(dataSize(sub_0.sub_0))
//     /* "source":174:196   */
//   pop(dataOffset(sub_0.sub_1))
//     /* "source":201:221   */
//   pop(dataSize(sub_0.sub_1))
//     /* "source":226:250   */
//   pop(dataOffset(sub_0.sub_0.sub_0))
//     /* "source":255:277   */
//   pop(dataSize(sub_0.sub_0.sub_0))
// stop
//
// sub_0: assembly {
//         /* "source":315:335   */
//       pop(dataOffset(sub_0))
//         /* "source":342:360   */
//       pop(dataSize(sub_0))
//         /* "source":367:387   */
//       pop(dataOffset(sub_1))
//         /* "source":394:412   */
//       pop(dataSize(sub_1))
//         /* "source":419:441   */
//       pop(dataOffset(sub_0.sub_0))
//         /* "source":448:468   */
//       pop(dataSize(sub_0.sub_0))
//     stop
//
//     sub_0: assembly {
//             /* "source":513:533   */
//           pop(dataOffset(sub_0))
//             /* "source":542:560   */
//           pop(dataSize(sub_0))
//         stop
//
//         sub_0: assembly {
//                 /* "source":613:622   */
//               invalid
//         }
//     }
//
//     sub_1: assembly {
//             /* "source":685:694   */
//           invalid
//     }
// }
// Bytecode: 60005060af50601f50602450604350600850608b50600150606750600150fe601350600850602350600150601b50600150fe600750600150fefe600750600150fefefe601350600850602350600150601b50600150fe600750600150fefe600750600150fefefe601350600850602350600150601b50600150fe600750600150fefe600750600150fefefe601350600850602350600150601b50600150fe600750600150fefe600750600150fefefe
// Opcodes: PUSH1 0x0 POP PUSH1 0xAF POP PUSH1 0x1F POP PUSH1 0x24 POP PUSH1 0x43 POP PUSH1 0x8 POP PUSH1 0x8B POP PUSH1 0x1 POP PUSH1 0x67 POP PUSH1 0x1 POP INVALID PUSH1 0x13 POP PUSH1 0x8 POP PUSH1 0x23 POP PUSH1 0x1 POP PUSH1 0x1B POP PUSH1 0x1 POP INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID INVALID PUSH1 0x13 POP PUSH1 0x8 POP PUSH1 0x23 POP PUSH1 0x1 POP PUSH1 0x1B POP PUSH1 0x1 POP INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID INVALID PUSH1 0x13 POP PUSH1 0x8 POP PUSH1 0x23 POP PUSH1 0x1 POP PUSH1 0x1B POP PUSH1 0x1 POP INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID INVALID PUSH1 0x13 POP PUSH1 0x8 POP PUSH1 0x23 POP PUSH1 0x1 POP PUSH1 0x1B POP PUSH1 0x1 POP INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID INVALID
// SourceMappings: 26:20:0:-:0;;51:18;;74:20;;99:18;;122:22;;149:20;;174:22;;201:20;;226:24;;255:22;
