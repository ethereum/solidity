object "a" {
  code { sstore(0, 1) }
}
// ----
// Assembly:
//     /* "source":36:37   */
//   0x01
//     /* "source":33:34   */
//   0x00
//     /* "source":26:38   */
//   sstore
//     /* "source":22:42   */
//   stop
// Bytecode: 60015f5500
// Opcodes: PUSH1 0x1 PUSH0 SSTORE STOP
// SourceMappings: 36:1:0:-:0;33;26:12;22:20
