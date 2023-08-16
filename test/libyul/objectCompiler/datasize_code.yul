object "a" {
  code { sstore(0, datasize("sub")) }
  object "sub" {
    code { sstore(0, 8) }
    data "data1" "Hello, World!"
  }
}
// ----
// Assembly:
//     /* "source":32:47   */
//   dataSize(sub_0)
//     /* "source":29:30   */
//   0x00
//     /* "source":22:48   */
//   sstore
//     /* "source":20:50   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":89:90   */
//       0x08
//         /* "source":86:87   */
//       0x00
//         /* "source":79:91   */
//       sstore
//         /* "source":77:93   */
//       stop
//     stop
//     data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// }
// Bytecode: 60065f5500fe
// Opcodes: PUSH1 0x6 PUSH0 SSTORE STOP INVALID
// SourceMappings: 32:15:0:-:0;29:1;22:26;20:30
