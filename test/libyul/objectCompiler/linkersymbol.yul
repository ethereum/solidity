object "a" {
    code {
        let addr := linkersymbol("contract/test.sol:L")
        mstore(128, shl(227, 0x18530aaf))
        let success := call(gas(), addr, 0, 128, 4, 128, 0)
    }
}
// ----
// Assembly:
//     /* "source":179:180   */
//   0x00
//     /* "source":174:177   */
//   0x80
//     /* "source":171:172   */
//   0x04
//     /* "source":44:79   */
//   dup2
//   dup4
//   linkerSymbol("f919ba91ac99f96129544b80b9516b27a80e376b9dc693819d0b18b7e0395612")
//     /* "source":109:119   */
//   0x18530aaf
//     /* "source":104:107   */
//   0xe3
//     /* "source":100:120   */
//   shl
//     /* "source":88:121   */
//   dup4
//   mstore
//     /* "source":150:155   */
//   gas
//     /* "source":145:181   */
//   call
//     /* "source":130:181   */
//   stop
// Bytecode: 5f6080600481837300000000000000000000000000000000000000006318530aaf60e31b83525af100
// Opcodes: PUSH0 PUSH1 0x80 PUSH1 0x4 DUP2 DUP4 PUSH20 0x0 PUSH4 0x18530AAF PUSH1 0xE3 SHL DUP4 MSTORE GAS CALL STOP
// SourceMappings: 179:1:0:-:0;174:3;171:1;44:35;;;109:10;104:3;100:20;88:33;;150:5;145:36;130:51
