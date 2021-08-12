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
// optimizationPreset: full
// ----
// Assembly:
//     /* "source":48:49   */
//   0x00
//     /* "source":35:50   */
//   dup1
//   calldataload
//     /* "source":107:127   */
//   sstore
//     /* "source":20:131   */
//   stop
// stop
//
// sub_0: assembly {
//         /* "source":188:189   */
//       0x00
//         /* "source":175:190   */
//       dup1
//       calldataload
//         /* "source":253:273   */
//       sstore
//         /* "source":158:279   */
//       stop
// }
// Bytecode: 600080355500fe
// Opcodes: PUSH1 0x0 DUP1 CALLDATALOAD SSTORE STOP INVALID
// SourceMappings: 48:1:0:-:0;35:15;;107:20;20:111
