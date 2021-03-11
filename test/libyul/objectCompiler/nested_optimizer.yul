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
// optimize: true
// ----
// Assembly:
//     /* "source":48:49   */
//   returndatasize
//   returndatasize
//     /* "source":35:50   */
//   calldataload
//     /* "source":107:127   */
//   sstore
// stop
//
// sub_0: assembly {
//         /* "source":188:189   */
//       returndatasize
//       returndatasize
//         /* "source":175:190   */
//       calldataload
//         /* "source":253:273   */
//       sstore
// }
// Bytecode: 3d3d3555fe
// Opcodes: RETURNDATASIZE RETURNDATASIZE CALLDATALOAD SSTORE INVALID
// SourceMappings: 48:1:0:-:0;;35:15;107:20
