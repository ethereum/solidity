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
//     /* "source":109:110   */
//   dup1
//     /* "source":38:39   */
//   dup2
//     /* "source":25:40   */
//   calldataload
//     /* "source":91:111   */
//   sstore
//     /* "source":12:113   */
//   pop
// Bytecode: 60008081355550
// Opcodes: PUSH1 0x0 DUP1 DUP2 CALLDATALOAD SSTORE POP
