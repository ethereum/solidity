object "a" {
    code {
        {
            mstore(0, auxdataloadn(0))
            return(0, 32)
        }
    }
    data "data1" hex"48656c6c6f2c20576f726c6421"
}

// ====
// bytecodeFormat: >=EOFv1
// EVMVersion: >=prague
// ----
// Assembly:
//     /* "source":56:71   */
//   auxdataloadn(0)
//     /* "source":53:54   */
//   0x00
//     /* "source":46:72   */
//   mstore
//     /* "source":95:97   */
//   0x20
//     /* "source":92:93   */
//   0x00
//     /* "source":85:98   */
//   return
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// Bytecode: ef0001010004020001000904002d000080ffffd1000d5f5260205ff348656c6c6f2c20576f726c6421
// Opcodes: 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP MULMOD DIV STOP 0x2D STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT DATALOADN 0xD PUSH0 MSTORE PUSH1 0x20 PUSH0 RETURN BASEFEE PUSH6 0x6C6C6F2C2057 PUSH16 0x726C6421000000000000000000000000
// SourceMappings: 56:15:0:-:0;53:1;46:26;95:2;92:1;85:13