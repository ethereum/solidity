object "a" {
  code {}
  // Unreferenced data is not added to the assembled bytecode.
  data "str" "Hello, World!"
  object "sub" {
    code { sstore(0, 1) }
    object "subsub" {
      code { sstore(2, 3) }
      data "str" hex"123456"
    }
  }
}
// ----
// Assembly:
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
//
// sub_0: assembly {
//         /* "source":153:154   */
//       0x01
//         /* "source":150:151   */
//       0x00
//         /* "source":143:155   */
//       sstore
//     stop
//
//     sub_0: assembly {
//             /* "source":203:204   */
//           0x03
//             /* "source":200:201   */
//           0x02
//             /* "source":193:205   */
//           sstore
//         stop
//         data_6adf031833174bbe4c85eafe59ddb54e6584648c2c962c6f94791ab49caa0ad4 123456
//     }
// }
// Bytecode: fe
// Opcodes: INVALID
