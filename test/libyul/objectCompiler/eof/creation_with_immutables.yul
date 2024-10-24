object "a" {
    code {
        mstore(0, eofcreate("b", 0, 0, 0, 0))
        return(0, 32)
    }

    object "b" {
        code {
            mstore(0, 0x1122334455667788990011223344556677889900112233445566778899001122)
            mstore(32, 0x1122334455667788990011223344556677889900112233445566778899001122)
            returncontract("c", 0, 64)
        }
        object "c" {
            code {
                let d0 := auxdataloadn(0)
                let d1 := auxdataloadn(32)

                mstore(0, d0)
                mstore(32, d1)

                return(0, 64)
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
//         /* "source":198:264   */
//       0x1122334455667788990011223344556677889900112233445566778899001122
//         /* "source":195:196   */
//       0x00
//         /* "source":188:265   */
//       mstore
//         /* "source":293:359   */
//       0x1122334455667788990011223344556677889900112233445566778899001122
//         /* "source":289:291   */
//       0x20
//         /* "source":282:360   */
//       mstore
//         /* "source":400:402   */
//       0x40
//         /* "source":397:398   */
//       0x00
//         /* "source":377:403   */
//       returcontract(0)
//     stop
//
//     sub_0: assembly {
//             /* "source":516:531   */
//           auxdataloadn(0)
//             /* "source":562:578   */
//           auxdataloadn(32)
//             /* "source":599:612   */
//           swap1
//             /* "source":606:607   */
//           0x00
//             /* "source":599:612   */
//           mstore
//             /* "source":640:642   */
//           0x20
//             /* "source":633:647   */
//           mstore
//             /* "source":678:680   */
//           0x40
//             /* "source":675:676   */
//           0x00
//             /* "source":668:681   */
//           return
//     }
// }
// Bytecode: ef0001010004020001000c030001008704000d000080ffff5f808080ec005f5260205ff3ef0001010004020001004c0300010023040000000080ffff7f11223344556677889900112233445566778899001122334455667788990011225f527f112233445566778899001122334455667788990011223344556677889900112260205260405fee00ef00010100040200010010040040000080ffffd10000d10020905f5260205260405ff348656c6c6f2c20576f726c6421
// Opcodes: 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP 0xC SUB STOP ADD STOP DUP8 DIV STOP 0xD STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT PUSH0 DUP1 DUP1 DUP1 EOFCREATE 0x0 PUSH0 MSTORE PUSH1 0x20 PUSH0 RETURN 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP 0x4C SUB STOP ADD STOP 0x23 DIV STOP STOP STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT PUSH32 0x1122334455667788990011223344556677889900112233445566778899001122 PUSH0 MSTORE PUSH32 0x1122334455667788990011223344556677889900112233445566778899001122 PUSH1 0x20 MSTORE PUSH1 0x40 PUSH0 RETURNCONTRACT 0x0 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP LT DIV STOP BLOCKHASH STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT DATALOADN 0x0 DATALOADN 0x20 SWAP1 PUSH0 MSTORE PUSH1 0x20 MSTORE PUSH1 0x40 PUSH0 RETURN BASEFEE PUSH6 0x6C6C6F2C2057 PUSH16 0x726C6421000000000000000000000000
// SourceMappings: 80:1:0:-:0;56:26;;;;53:1;46:37;106:2;103:1;96:13