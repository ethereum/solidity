object "a" {
    code {
        {
            auxdatastore(0, 32, 0x1122334455667788990011223344556677889900112233445566778899001122)
            return(0, 32)
        }
    }

    object "b" {
        code {
            {
                mstore(0, auxdataloadn(32))
                return(0, 32)
            }
        }
   }

    data "data1" hex"48656c6c6f2c20576f726c6421"
}

// ====
// bytecodeFormat: >=EOFv1
// EVMVersion: >=prague
// ----
// Assembly:
//     /* "source":66:132   */
//   0x1122334455667788990011223344556677889900112233445566778899001122
//     /* "source":62:64   */
//   0x20
//     /* "source":59:60   */
//   0x00
//     /* "source":46:133   */
//   auxdatastore()
//     /* "source":156:158   */
//   0x20
//     /* "source":153:154   */
//   0x00
//     /* "source":146:159   */
//   return
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
//
// sub_0: assembly {
//         /* "source":248:264   */
//       auxdataloadn(32)
//         /* "source":245:246   */
//       0x00
//         /* "source":238:265   */
//       mstore
//         /* "source":292:294   */
//       0x20
//         /* "source":289:290   */
//       0x00
//         /* "source":282:295   */
//       return
// }
// Bytecode: ef0001010004020001002a030001001c04000d000080ffff7f112233445566778899001122334455667788990011223344556677889900112260205f015260205ff3ef00010100040200010009040040000080ffffd100205f5260205ff348656c6c6f2c20576f726c6421
// Opcodes: 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP 0x2A SUB STOP ADD STOP SHR DIV STOP 0xD STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT PUSH32 0x1122334455667788990011223344556677889900112233445566778899001122 PUSH1 0x20 PUSH0 ADD MSTORE PUSH1 0x20 PUSH0 RETURN 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP MULMOD DIV STOP BLOCKHASH STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT DATALOADN 0x20 PUSH0 MSTORE PUSH1 0x20 PUSH0 RETURN BASEFEE PUSH6 0x6C6C6F2C2057 PUSH16 0x726C6421000000000000000000000000
// SourceMappings: 66:66:0:-:0;62:2;59:1;46:87;156:2;153:1;146:13