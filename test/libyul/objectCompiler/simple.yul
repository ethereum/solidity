{
  sstore(0, 1)
}
// ====
// EVMVersion: >=shanghai
// ----
// Assembly:
//     /* "source":41:42   */
//   0x01
//     /* "source":38:39   */
//   0x00
//     /* "source":31:43   */
//   sstore
//     /* "source":27:47   */
//   stop
// Bytecode: 60015f5500
// Opcodes: PUSH1 0x1 PUSH0 SSTORE STOP
// SourceMappings: 41:1:0:-:0;38;31:12;27:20
