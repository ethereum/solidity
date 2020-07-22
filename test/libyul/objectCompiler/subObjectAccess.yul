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

  data "data1" "Hello, World!"

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
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
//
// sub_0: assembly {
//         /* "source":347:367   */
//       pop(dataOffset(sub_0))
//         /* "source":374:392   */
//       pop(dataSize(sub_0))
//         /* "source":399:419   */
//       pop(dataOffset(sub_1))
//         /* "source":426:444   */
//       pop(dataSize(sub_1))
//         /* "source":451:473   */
//       pop(dataOffset(sub_0.sub_0))
//         /* "source":480:500   */
//       pop(dataSize(sub_0.sub_0))
//     stop
//
//     sub_0: assembly {
//             /* "source":545:565   */
//           pop(dataOffset(sub_0))
//             /* "source":574:592   */
//           pop(dataSize(sub_0))
//         stop
//
//         sub_0: assembly {
//                 /* "source":645:654   */
//               invalid
//         }
//     }
//
//     sub_1: assembly {
//             /* "source":717:726   */
//           invalid
//     }
// }
// Bytecode: 600050604650601f50601d50603e50600850603d50600150603c50600150fe601350600850601b50600150601c50600150fe600750600150fefefefefefe600750600150fefe
// Opcodes: PUSH1 0x0 POP PUSH1 0x46 POP PUSH1 0x1F POP PUSH1 0x1D POP PUSH1 0x3E POP PUSH1 0x8 POP PUSH1 0x3D POP PUSH1 0x1 POP PUSH1 0x3C POP PUSH1 0x1 POP INVALID PUSH1 0x13 POP PUSH1 0x8 POP PUSH1 0x1B POP PUSH1 0x1 POP PUSH1 0x1C POP PUSH1 0x1 POP INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID INVALID INVALID INVALID INVALID PUSH1 0x7 POP PUSH1 0x1 POP INVALID INVALID
// SourceMappings: 26:20:0:-:0;;51:18;;74:20;;99:18;;122:22;;149:20;;174:22;;201:20;;226:24;;255:22;
