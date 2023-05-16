object "a" {
    code {
        setimmutable(
            0,
            "long___name___that___definitely___exceeds___the___thirty___two___byte___limit",
             0x1234567890123456789012345678901234567890
        )
    }
}
// ----
// Assembly:
//     /* "source":167:209   */
//   0x1234567890123456789012345678901234567890
//     /* "source":58:59   */
//   0x00
//     /* "source":32:219   */
//   assignImmutable("0x85a5b1db611c82c46f5fa18e39ae218397536256c451e5de155a86de843a9ad6")
//     /* "source":22:225   */
//   stop
// Bytecode: 7312345678901234567890123456789012345678905f505000
// Opcodes: PUSH20 0x1234567890123456789012345678901234567890 PUSH0 POP POP STOP
// SourceMappings: 167:42:0:-:0;58:1;32:187;;22:203
