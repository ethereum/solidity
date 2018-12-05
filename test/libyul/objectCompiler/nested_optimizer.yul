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
//     /* "source":137:138   */
//   dup1
//     /* "source":60:61   */
//   dup2
//     /* "source":47:62   */
//   calldataload
//     /* "source":119:139   */
//   sstore
//     /* "source":32:143   */
//   pop
// stop
//
// sub_0: assembly {
//         /* "source":200:201   */
//       0x00
//         /* "source":283:284   */
//       dup1
//         /* "source":200:201   */
//       dup2
//         /* "source":187:202   */
//       calldataload
//         /* "source":265:285   */
//       sstore
//         /* "source":170:291   */
//       pop
// }
// Bytecode: 60008081355550fe
// Opcodes: PUSH1 0x0 DUP1 DUP2 CALLDATALOAD SSTORE POP INVALID
