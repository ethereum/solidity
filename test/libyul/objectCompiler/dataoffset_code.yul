object "a" {
  code { sstore(0, dataoffset("sub")) }
  object "sub" {
    code { sstore(0, 8) }
    data "data1" "Hello, World!"
  }
}
// ----
// Assembly:
//     /* "source":22:50   */
//   dataOffset(sub_0)
//     /* "source":29:30   */
//   0x00
//     /* "source":22:50   */
//   sstore
// stop
//
// sub_0: assembly {
//         /* "source":91:92   */
//       0x08
//         /* "source":88:89   */
//       0x00
//         /* "source":81:93   */
//       sstore
//     stop
//     data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// }
// Bytecode: 6006600055fe6008600055fe
// Opcodes: PUSH1 0x6 PUSH1 0x0 SSTORE INVALID PUSH1 0x8 PUSH1 0x0 SSTORE INVALID
