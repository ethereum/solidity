{
  let x := calldataload(0)
  let y := calldataload(0)
  let z := sub(y, x)
  sstore(add(x, 0), z)
}
// ====
// optimize: true
// ----
// Assembly:
//     /* "source":26:27   */
//   returndatasize
//   returndatasize
//     /* "source":13:28   */
//   calldataload
//     /* "source":79:99   */
//   sstore
// Bytecode: 3d3d3555
// Opcodes: RETURNDATASIZE RETURNDATASIZE CALLDATALOAD SSTORE
// SourceMappings: 26:1:0:-:0;;13:15;79:20
