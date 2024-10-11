object "a" {
  code { sstore(0, datasize("sub")) }
  object "sub" {
    code { sstore(0, 8) }
    data "data1" "Hello, World!"
  }
}
// ====
// EVMVersion: >=shanghai
// ----
// Assembly:
//     /* "source":44:59   */
//   dataSize(sub_0)
//     /* "source":41:42   */
//   0x00
//     /* "source":34:60   */
//   sstore
//     /* "source":22:68   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":115:116   */
//       0x08
//         /* "source":112:113   */
//       0x00
//         /* "source":105:117   */
//       sstore
//         /* "source":101:121   */
//       stop
//     stop
//     data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// }
// Bytecode: 60065f5500fe
// Opcodes: PUSH1 0x6 PUSH0 SSTORE STOP INVALID
// SourceMappings: 44:15:0:-:0;41:1;34:26;22:46
