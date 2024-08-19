object "a" {
  code { sstore(0, dataoffset("sub")) }
  object "sub" {
    code { sstore(0, 8) }
    data "data1" "Hello, World!"
  }
}
// ----
// Assembly:
//     /* "source":44:61   */
//   dataOffset(sub_0)
//     /* "source":41:42   */
//   0x00
//     /* "source":34:62   */
//   sstore
//     /* "source":22:70   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":117:118   */
//       0x08
//         /* "source":114:115   */
//       0x00
//         /* "source":107:119   */
//       sstore
//         /* "source":103:123   */
//       stop
//     stop
//     data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// }
// Bytecode: 60065f5500fe60085f5500fe
// Opcodes: PUSH1 0x6 PUSH0 SSTORE STOP INVALID PUSH1 0x8 PUSH0 SSTORE STOP INVALID
// SourceMappings: 44:17:0:-:0;41:1;34:28;22:48
