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
//     /* "source":57:72   */
//   dataSize(sub_0)
//     /* "source":38:55   */
//   dataOffset(sub_0)
//     /* "source":35:36   */
//   0x00
//     /* "source":26:73   */
//   codecopy
//     /* "source":88:103   */
//   dataSize(sub_0)
//     /* "source":85:86   */
//   0x00
//     /* "source":78:104   */
//   return
// stop
//
// sub_0: assembly {
//         /* "source":153:170   */
//       0x00
//         /* "source":150:151   */
//       dup1
//         /* "source":143:171   */
//       sstore
//         /* "source":188:205   */
//       0x0d
//         /* "source":185:186   */
//       0x00
//         /* "source":178:206   */
//       mstore
//         /* "source":135:212   */
//       stop
//     stop
//     data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// }
// Bytecode: 6009600b5f3960095ff3fe5f8055600d5f5200fe
// Opcodes: PUSH1 0x9 PUSH1 0xB PUSH0 CODECOPY PUSH1 0x9 PUSH0 RETURN INVALID PUSH0 DUP1 SSTORE PUSH1 0xD PUSH0 MSTORE STOP INVALID
// SourceMappings: 57:15:0:-:0;38:17;35:1;26:47;88:15;85:1;78:26
