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
// ====
// optimize: true
// ----
// Assembly:
//     /* "source":48:49   */
//   0x00
//   0x00
//     /* "source":35:50   */
//   calldataload
//     /* "source":107:127   */
//   sstore
// stop
//
// sub_0: assembly {
//         /* "source":188:189   */
//       0x00
//       0x00
//         /* "source":175:190   */
//       calldataload
//         /* "source":253:273   */
//       sstore
// }
// Bytecode: 600060003555fe
// Opcodes: PUSH1 0x0 PUSH1 0x0 CALLDATALOAD SSTORE INVALID
