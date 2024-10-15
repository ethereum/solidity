object "a" {
  code {}
  // Unreferenced data is not added to the assembled bytecode.
  data "str" "Hello, World!"
  object "sub" { code { sstore(0, 1) } }
}
// ====
// EVMVersion: >=constantinople
// ----
// Assembly:
//     /* "source":22:29   */
//   stop
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
//
// sub_0: assembly {
//         /* "source":123:124   */
//       0x01
//         /* "source":120:121   */
//       0x00
//         /* "source":113:125   */
//       sstore
//         /* "source":109:129   */
//       stop
// }
// Bytecode: 00fe
// Opcodes: STOP INVALID
// SourceMappings: 22:7:0:-:0
