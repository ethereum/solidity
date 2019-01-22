// optimize
object "a" {
  code {
    let x := calldataload(0)
    let y := calldataload(0)
    let z := sub(y, x)
    sstore(add(x, 0), z)
  }
  object "sub" {
    code {
      let x := calldataload(0)
      let y := calldataload(0)
      let z := sub(y, x)
      sstore(add(x, 0), z)
    }
  }
}
// ----
// Assembly:
//     /* "source":60:61   */
//   0x00
//   0x00
//     /* "source":47:62   */
//   calldataload
//     /* "source":119:139   */
//   sstore
// stop
//
// sub_0: assembly {
//         /* "source":200:201   */
//       0x00
//       0x00
//         /* "source":187:202   */
//       calldataload
//         /* "source":265:285   */
//       sstore
// }
// Bytecode: 600060003555fe
// Opcodes: PUSH1 0x0 PUSH1 0x0 CALLDATALOAD SSTORE INVALID
