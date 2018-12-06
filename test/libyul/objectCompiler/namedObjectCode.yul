object "a" {
  code { sstore(0, 1) }
}
// ----
// Assembly:
//     /* "source":32:33   */
//   0x01
//     /* "source":29:30   */
//   0x00
//     /* "source":22:34   */
//   sstore
// Bytecode: 6001600055
// Opcodes: PUSH1 0x1 PUSH1 0x0 SSTORE
