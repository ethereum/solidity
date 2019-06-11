{
  let x := calldataload(0)
  let y := calldataload(0)
  let z := sub(y, x)
  sstore(add(x, 0), z)
}
// ====
// optimize: true
// ----
// Assembly:
//     /* "source":26:27   */
//   0x00
//   0x00
//     /* "source":13:28   */
//   calldataload
//     /* "source":79:99   */
//   sstore
// Bytecode: 600060003555
// Opcodes: PUSH1 0x0 PUSH1 0x0 CALLDATALOAD SSTORE
