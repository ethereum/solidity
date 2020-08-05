object "a" {
    code {
        setimmutable(
            "long___name___that___definitely___exceeds___the___thirty___two___byte___limit",
             0x1234567890123456789012345678901234567890
        )
    }
}
// ----
// Assembly:
//     /* "source":152:194   */
//   0x1234567890123456789012345678901234567890
//     /* "source":32:204   */
//   assignImmutable("0x85a5b1db611c82c46f5fa18e39ae218397536256c451e5de155a86de843a9ad6")
// Bytecode: 73123456789012345678901234567890123456789050
// Opcodes: PUSH20 0x1234567890123456789012345678901234567890 POP
// SourceMappings: 152:42:0:-:0;32:172
