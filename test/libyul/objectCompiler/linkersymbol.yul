object "a" {
    code {
        let addr := linkersymbol("contract/test.sol:L")
        mstore(128, shl(227, 0x18530aaf))
        let success := call(gas(), addr, 0, 128, 4, 128, 0)
    }
}
// ----
// Assembly:
//     /* "source":44:79   */
//   linkerSymbol("f919ba91ac99f96129544b80b9516b27a80e376b9dc693819d0b18b7e0395612")
//     /* "source":109:119   */
//   0x18530aaf
//     /* "source":104:107   */
//   0xe3
//     /* "source":100:120   */
//   shl
//     /* "source":95:98   */
//   0x80
//     /* "source":88:121   */
//   mstore
//     /* "source":179:180   */
//   0x00
//     /* "source":174:177   */
//   0x80
//     /* "source":171:172   */
//   0x04
//     /* "source":166:169   */
//   0x80
//     /* "source":163:164   */
//   0x00
//     /* "source":157:161   */
//   dup6
//     /* "source":150:155   */
//   gas
//     /* "source":145:181   */
//   call
//     /* "source":22:187   */
//   pop
//   pop
// Bytecode: 7300000000000000000000000000000000000000006318530aaf60e31b60805260006080600460806000855af15050
// Opcodes: PUSH20 0x0 PUSH4 0x18530AAF PUSH1 0xE3 SHL PUSH1 0x80 MSTORE PUSH1 0x0 PUSH1 0x80 PUSH1 0x4 PUSH1 0x80 PUSH1 0x0 DUP6 GAS CALL POP POP
// SourceMappings: 44:35:0:-:0;109:10;104:3;100:20;95:3;88:33;179:1;174:3;171:1;166:3;163:1;157:4;150:5;145:36;22:165;
