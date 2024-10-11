object "a" {
    code {
        setimmutable(
            0,
            "long___name___that___definitely___exceeds___the___thirty___two___byte___limit",
             0x1234567890123456789012345678901234567890
        )
    }
}
// ====
// EVMVersion: >=shanghai
// ----
// Assembly:
//     /* "source":143:185   */
//   0x1234567890123456789012345678901234567890
//     /* "source":59:60   */
//   0x00
//     /* "source":46:186   */
//   assignImmutable("0x85a5b1db611c82c46f5fa18e39ae218397536256c451e5de155a86de843a9ad6")
//     /* "source":22:202   */
//   stop
// Bytecode: 7312345678901234567890123456789012345678905f505000
// Opcodes: PUSH20 0x1234567890123456789012345678901234567890 PUSH0 POP POP STOP
// SourceMappings: 143:42:0:-:0;59:1;46:140;;22:180
