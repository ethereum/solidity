object "a" {
    code {
        let addr := linkersymbol("contract/test.sol:L")
        mstore(128, shl(227, 0x18530aaf))
        let success := call(gas(), addr, 0, 128, 4, 128, 0)
    }
}
// ====
// EVMVersion: >=shanghai
// ----
// Assembly:
//     /* "source":190:191   */
//   0x00
//     /* "source":185:188   */
//   0x80
//     /* "source":182:183   */
//   0x04
//     /* "source":58:93   */
//   dup2
//   dup4
//   linkerSymbol("f919ba91ac99f96129544b80b9516b27a80e376b9dc693819d0b18b7e0395612")
//     /* "source":127:137   */
//   0x18530aaf
//     /* "source":122:125   */
//   0xe3
//     /* "source":118:138   */
//   shl
//     /* "source":106:139   */
//   dup4
//   mstore
//     /* "source":161:166   */
//   gas
//     /* "source":156:192   */
//   call
//     /* "source":152:193   */
//   stop
// Bytecode: 5f6080600481837300000000000000000000000000000000000000006318530aaf60e31b83525af100
// Opcodes: PUSH0 PUSH1 0x80 PUSH1 0x4 DUP2 DUP4 PUSH20 0x0 PUSH4 0x18530AAF PUSH1 0xE3 SHL DUP4 MSTORE GAS CALL STOP
// SourceMappings: 190:1:0:-:0;185:3;182:1;58:35;;;127:10;122:3;118:20;106:33;;161:5;156:36;152:41
