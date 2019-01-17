// optimize
{
  let x := calldataload(0)
  let y := calldataload(0)
  let z := sub(y, x)
  sstore(add(x, 0), z)
}
// ----
// Assembly:
//     /* "source":38:39   */
//   0x00
//   0x00
//     /* "source":25:40   */
//   calldataload
//     /* "source":91:111   */
//   sstore
// Bytecode: 600060003555
// Opcodes: PUSH1 0x0 PUSH1 0x0 CALLDATALOAD SSTORE
