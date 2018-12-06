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
//     /* "source":26:73   */
//   dataSize(sub_0)
//   dataOffset(sub_0)
//     /* "source":35:36   */
//   0x00
//     /* "source":26:73   */
//   codecopy
//     /* "source":78:104   */
//   dataSize(sub_0)
//     /* "source":85:86   */
//   0x00
//     /* "source":78:104   */
//   return
// stop
//
// sub_0: assembly {
//         /* "source":143:171   */
//       0x00
//         /* "source":150:151   */
//       0x00
//         /* "source":143:171   */
//       sstore
//         /* "source":178:206   */
//       0x0d
//         /* "source":185:186   */
//       0x00
//         /* "source":178:206   */
//       mstore
//     stop
//     data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// }
// Bytecode: 600b600d600039600b6000f3fe6000600055600d600052fe
// Opcodes: PUSH1 0xB PUSH1 0xD PUSH1 0x0 CODECOPY PUSH1 0xB PUSH1 0x0 RETURN INVALID PUSH1 0x0 PUSH1 0x0 SSTORE PUSH1 0xD PUSH1 0x0 MSTORE INVALID
