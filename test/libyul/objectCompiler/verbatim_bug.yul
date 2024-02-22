object "a" {
    code {
        let dummy := 0xAABBCCDDEEFF
        let input := sload(0)
        let output

        switch input
        case 0x00 {
            // Note that due to a bug the following disappeared from the assembly output.
            output := verbatim_1i_1o(hex"506000", dummy)
        }
        case 0x01 {
            output := 1
        }
        case 0x02 {
            output := verbatim_1i_1o(hex"506002", dummy)
        }
        case 0x03 {
            output := 3
        }

        sstore(0, output)
    }
}
// ====
// optimizationPreset: full
// ----
// Assembly:
//     /* "source":87:88   */
//   0x00
//     /* "source":81:89   */
//   sload
//     /* "source":87:88   */
//   0x00
//     /* "source":118:502   */
//   swap1
//     /* "source":139:307   */
//   dup1
//     /* "source":144:148   */
//   0x00
//     /* "source":139:307   */
//   eq
//   tag_1
//   jumpi
//     /* "source":316:361   */
//   dup1
//     /* "source":321:325   */
//   0x01
//     /* "source":316:361   */
//   eq
//   tag_3
//   jumpi
//     /* "source":370:448   */
//   dup1
//     /* "source":375:379   */
//   0x02
//     /* "source":370:448   */
//   eq
//   tag_5
//   jumpi
//     /* "source":462:466   */
//   0x03
//     /* "source":457:502   */
//   eq
//   tag_7
//   jumpi
//     /* "source":87:88   */
//   0x00
//     /* "source":512:529   */
//   sstore
//     /* "source":118:502   */
//   stop
//     /* "source":467:502   */
// tag_7:
//     /* "source":481:492   */
//   pop
//     /* "source":491:492   */
//   0x03
//     /* "source":87:88   */
//   0x00
//     /* "source":512:529   */
//   sstore
//     /* "source":118:502   */
//   stop
//     /* "source":380:448   */
// tag_5:
//     /* "source":404:438   */
//   pop
//   pop
//     /* "source":45:59   */
//   0xaabbccddeeff
//     /* "source":404:438   */
//   verbatimbytecode_506002
//     /* "source":87:88   */
//   0x00
//     /* "source":512:529   */
//   sstore
//     /* "source":118:502   */
//   stop
//     /* "source":326:361   */
// tag_3:
//     /* "source":340:351   */
//   pop
//   pop
//     /* "source":350:351   */
//   0x01
//     /* "source":87:88   */
//   0x00
//     /* "source":512:529   */
//   sstore
//     /* "source":118:502   */
//   stop
//     /* "source":149:307   */
// tag_1:
//     /* "source":263:297   */
//   pop
//   pop
//     /* "source":45:59   */
//   0xaabbccddeeff
//     /* "source":263:297   */
//   verbatimbytecode_506000
//     /* "source":87:88   */
//   0x00
//     /* "source":512:529   */
//   sstore
//     /* "source":118:502   */
//   stop
// Bytecode: 5f545f90805f1460405780600114603857806002146028576003146021575f55005b5060035f55005b505065aabbccddeeff5060025f55005b505060015f55005b505065aabbccddeeff5060005f5500
// Opcodes: PUSH0 SLOAD PUSH0 SWAP1 DUP1 PUSH0 EQ PUSH1 0x40 JUMPI DUP1 PUSH1 0x1 EQ PUSH1 0x38 JUMPI DUP1 PUSH1 0x2 EQ PUSH1 0x28 JUMPI PUSH1 0x3 EQ PUSH1 0x21 JUMPI PUSH0 SSTORE STOP JUMPDEST POP PUSH1 0x3 PUSH0 SSTORE STOP JUMPDEST POP POP PUSH6 0xAABBCCDDEEFF POP PUSH1 0x2 PUSH0 SSTORE STOP JUMPDEST POP POP PUSH1 0x1 PUSH0 SSTORE STOP JUMPDEST POP POP PUSH6 0xAABBCCDDEEFF POP PUSH1 0x0 PUSH0 SSTORE STOP
// SourceMappings: 87:1:0:-:0;81:8;87:1;118:384;139:168;144:4;139:168;;;316:45;321:4;316:45;;;370:78;375:4;370:78;;;462:4;457:45;;;87:1;512:17;118:384;467:35;481:11;491:1;87;512:17;118:384;380:68;404:34;;45:14;404:34;87:1;512:17;118:384;326:35;340:11;;350:1;87;512:17;118:384;149:158;263:34;;45:14;263:34;87:1;512:17;118:384
