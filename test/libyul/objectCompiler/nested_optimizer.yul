object "a" {
  code {
    let x := calldataload(0)
    let y := calldataload(0)
    let z := sub(y, x)
    sstore(add(x, 0), z)
  }
  object "sub" {
    code {
      let x := calldataload(0)
      let y := calldataload(0)
      let z := sub(y, x)
      sstore(add(x, 0), z)
    }
  }
}
// ====
// EVMVersion: >=shanghai
// optimizationPreset: full
// ----
// Assembly:
//     /* "source":58:59   */
//   0x00
//     /* "source":41:56   */
//   dup1
//   calldataload
//     /* "source":34:60   */
//   sstore
//     /* "source":22:68   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":141:142   */
//       0x00
//         /* "source":124:139   */
//       dup1
//       calldataload
//         /* "source":117:143   */
//       sstore
//         /* "source":101:155   */
//       stop
// }
// Bytecode: 5f80355500fe
// Opcodes: PUSH0 DUP1 CALLDATALOAD SSTORE STOP INVALID
// SourceMappings: 58:1:0:-:0;41:15;;34:26;22:46
