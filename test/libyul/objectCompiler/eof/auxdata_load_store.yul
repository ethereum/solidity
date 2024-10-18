object "a" {
    code {
        {
            auxdatastore(0, 0, 0x1122334455667788990011223344556677889900112233445566778899001122)
            returncontract("b", 0, 32)
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
//     /* "source":65:131   */
//   0x1122334455667788990011223344556677889900112233445566778899001122
//     /* "source":62:63   */
//   0x00
//     /* "source":46:132   */
//   dup1
//   auxdatastore()
//     /* "source":168:170   */
//   0x20
//     /* "source":165:166   */
//   0x00
//     /* "source":145:171   */
//   returcontract(0)
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
//
// sub_0: assembly {
//         /* "source":286:302   */
//       auxdataloadn(32)
//         /* "source":283:284   */
//       0x00
//         /* "source":276:303   */
//       mstore
//         /* "source":330:332   */
//       0x20
//         /* "source":327:328   */
//       0x00
//         /* "source":320:333   */
//       return
// }
// Bytecode: ef0001010004020001002a030001001c04000d000080ffff7f11223344556677889900112233445566778899001122334455667788990011225f80015260205fee00ef00010100040200010009040040000080ffffd100205f5260205ff348656c6c6f2c20576f726c6421
// Opcodes: 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP 0x2A SUB STOP ADD STOP SHR DIV STOP 0xD STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT PUSH32 0x1122334455667788990011223344556677889900112233445566778899001122 PUSH0 DUP1 ADD MSTORE PUSH1 0x20 PUSH0 RETURNCONTRACT 0x0 0xEF STOP ADD ADD STOP DIV MUL STOP ADD STOP MULMOD DIV STOP BLOCKHASH STOP STOP DUP1 SELFDESTRUCT SELFDESTRUCT DATALOADN 0x20 PUSH0 MSTORE PUSH1 0x20 PUSH0 RETURN BASEFEE PUSH6 0x6C6C6F2C2057 PUSH16 0x726C6421000000000000000000000000
// SourceMappings: 65:66:0:-:0;62:1;46:86;;168:2;165:1;145:26