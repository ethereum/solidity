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
//     /* "source":20:36   */
//   stop
// Bytecode: 60015f5500
// Opcodes: PUSH1 0x1 PUSH0 SSTORE STOP
// SourceMappings: 32:1:0:-:0;29;22:12;20:16
