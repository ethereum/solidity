object "a" {
  code {
    datacopy(0, dataoffset("sub"), datasize("sub"))
    return(0, datasize("sub"))
  }
  object "sub" {
    code {
      sstore(0, dataoffset("sub"))
      mstore(0, datasize("data1"))
    }
    data "data1" "Hello, World!"
  }
}
// ----
// Assembly:
//     /* "source":77:92   */
//   dataSize(sub_0)
//     /* "source":58:75   */
//   dataOffset(sub_0)
//     /* "source":55:56   */
//   0x00
//     /* "source":46:93   */
//   codecopy
//     /* "source":116:131   */
//   dataSize(sub_0)
//     /* "source":113:114   */
//   0x00
//     /* "source":106:132   */
//   return
// stop
//
// sub_0: assembly {
//         /* "source":223:240   */
//       0x00
//         /* "source":220:221   */
//       0x00
//         /* "source":213:241   */
//       sstore
//         /* "source":268:285   */
//       0x0d
//         /* "source":265:266   */
//       0x00
//         /* "source":258:286   */
//       mstore
//         /* "source":181:310   */
//       stop
//     stop
//     data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// }
// Bytecode: 6009600b5f3960095ff3fe5f5f55600d5f5200fe
// Opcodes: PUSH1 0x9 PUSH1 0xB PUSH0 CODECOPY PUSH1 0x9 PUSH0 RETURN INVALID PUSH0 PUSH0 SSTORE PUSH1 0xD PUSH0 MSTORE STOP INVALID
// SourceMappings: 77:15:0:-:0;58:17;55:1;46:47;116:15;113:1;106:26
