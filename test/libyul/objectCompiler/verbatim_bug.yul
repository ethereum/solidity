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
// EVMVersion: >=shanghai
// optimizationPreset: full
// ----
// Assembly:
//     /* "source":65:66   */
//   0x00
//     /* "source":59:67   */
//   dup1
//   sload
//     /* "source":133:225   */
//   dup1
//   iszero
//   tag_1
//   jumpi
//     /* "source":238:263   */
//   dup1
//     /* "source":243:247   */
//   0x01
//     /* "source":238:263   */
//   eq
//   tag_3
//   jumpi
//     /* "source":276:368   */
//   dup1
//     /* "source":281:285   */
//   0x02
//     /* "source":276:368   */
//   eq
//   tag_5
//   jumpi
//     /* "source":386:390   */
//   0x03
//     /* "source":381:406   */
//   eq
//   tag_7
//   jumpi
//     /* "source":426:427   */
//   0x00
//     /* "source":419:436   */
//   sstore
//     /* "source":108:406   */
//   stop
//     /* "source":391:406   */
// tag_7:
//     /* "source":393:404   */
//   pop
//     /* "source":403:404   */
//   0x03
//     /* "source":426:427   */
//   0x00
//     /* "source":419:436   */
//   sstore
//     /* "source":108:406   */
//   stop
//     /* "source":286:368   */
// tag_5:
//     /* "source":314:354   */
//   pop
//   pop
//     /* "source":339:353   */
//   0xaabbccddeeff
//     /* "source":314:354   */
//   verbatimbytecode_506002
//     /* "source":426:427   */
//   0x00
//     /* "source":419:436   */
//   sstore
//     /* "source":108:406   */
//   stop
//     /* "source":248:263   */
// tag_3:
//     /* "source":250:261   */
//   pop
//   pop
//     /* "source":260:261   */
//   0x01
//     /* "source":426:427   */
//   0x00
//     /* "source":419:436   */
//   sstore
//     /* "source":108:406   */
//   stop
//     /* "source":143:225   */
// tag_1:
//     /* "source":171:211   */
//   pop
//   pop
//     /* "source":196:210   */
//   0xaabbccddeeff
//     /* "source":171:211   */
//   verbatimbytecode_506000
//     /* "source":426:427   */
//   0x00
//     /* "source":419:436   */
//   sstore
//     /* "source":108:406   */
//   stop
// Bytecode: 5f80548015603e578060011460365780600214602657600314601f575f55005b5060035f55005b505065aabbccddeeff5060025f55005b505060015f55005b505065aabbccddeeff5060005f5500
// Opcodes: PUSH0 DUP1 SLOAD DUP1 ISZERO PUSH1 0x3E JUMPI DUP1 PUSH1 0x1 EQ PUSH1 0x36 JUMPI DUP1 PUSH1 0x2 EQ PUSH1 0x26 JUMPI PUSH1 0x3 EQ PUSH1 0x1F JUMPI PUSH0 SSTORE STOP JUMPDEST POP PUSH1 0x3 PUSH0 SSTORE STOP JUMPDEST POP POP PUSH6 0xAABBCCDDEEFF POP PUSH1 0x2 PUSH0 SSTORE STOP JUMPDEST POP POP PUSH1 0x1 PUSH0 SSTORE STOP JUMPDEST POP POP PUSH6 0xAABBCCDDEEFF POP PUSH1 0x0 PUSH0 SSTORE STOP
// SourceMappings: 65:1:0:-:0;59:8;;133:92;;;;238:25;243:4;238:25;;;276:92;281:4;276:92;;;386:4;381:25;;;426:1;419:17;108:298;391:15;393:11;403:1;426;419:17;108:298;286:82;314:40;;339:14;314:40;426:1;419:17;108:298;248:15;250:11;;260:1;426;419:17;108:298;143:82;171:40;;196:14;171:40;426:1;419:17;108:298
