object "a" {
    code {
        mstore(0, eofcreate("b", 0, 0, 0, 0))
        return(0, 32)
    }

    object "b" {
        code {
            auxdatastore(0, 0, 0x1122334455667788990011223344556677889900112233445566778899001122)
            auxdatastore(0, 32, 0x1234567890123456789012345678901234567890123456789012345678901234)
            returncontract("c", 0, 64)
        }
        object "c" {
            code {
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
//     /* "source":80:81   */
//   0x00
//     /* "source":56:82   */
//   dup1
//   dup1
//   dup1
//   eofcreate(0)
//     /* "source":53:54   */
//   0x00
//     /* "source":46:83   */
//   mstore
//     /* "source":106:108   */
//   0x20
//     /* "source":103:104   */
//   0x00
//     /* "source":96:109   */
//   return
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
//
// sub_0: assembly {
//         /* "source":207:273   */
//       0x1122334455667788990011223344556677889900112233445566778899001122
//         /* "source":204:205   */
//       0x00
//         /* "source":188:274   */
//       dup1
//       auxdatastore()
//         /* "source":311:377   */
//       0x1234567890123456789012345678901234567890123456789012345678901234
//         /* "source":307:309   */
//       0x20
//         /* "source":304:305   */
//       0x00
//         /* "source":291:378   */
//       auxdatastore()
//         /* "source":418:420   */
//       0x40
//         /* "source":415:416   */
//       0x00
//         /* "source":395:421   */
//       returcontract(0)
//     stop
//
//     sub_0: assembly {
//             /* "source":484:491   */
//           stop
//     }
// }
// Bytecode: ef0001010004020001000c030001007c04000d000080ffff5f808080ec005f5260205ff3ef000101000402000100500300010014040000000080ffff7f11223344556677889900112233445566778899001122334455667788990011225f8001527f123456789012345678901234567890123456789012345678901234567890123460205f015260405fee00ef00010100040200010001040000000080ffff0048656c6c6f2c20576f726c6421
// Opcodes: 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP 0xC SUB STOP ADD STOP PUSH29 0x4000D000080FFFF5F808080EC005F5260205FF3EF0001010004020001 STOP POP SUB STOP ADD STOP EQ DIV STOP STOP STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT PUSH32 0x1122334455667788990011223344556677889900112233445566778899001122 PUSH0 DUP1 ADD MSTORE PUSH32 0x1234567890123456789012345678901234567890123456789012345678901234 PUSH1 0x20 PUSH0 ADD MSTORE PUSH1 0x40 PUSH0 RETURNCONTRACT 0x0 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP ADD DIV STOP STOP STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT STOP BASEFEE PUSH6 0x6C6C6F2C2057 PUSH16 0x726C6421000000000000000000000000
// SourceMappings: 80:1:0:-:0;56:26;;;;53:1;46:37;106:2;103:1;96:13
