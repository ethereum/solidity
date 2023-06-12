{
  let x := calldataload(0)
  let y := calldataload(0)
  let z := sub(y, x)
  sstore(add(x, 0), z)
}
// ====
// optimizationPreset: full
// ----
// Assembly:
//     /* "source":67:76   */
//   0x00
//     /* "source":13:28   */
//   dup1
//   calldataload
//     /* "source":79:99   */
//   sstore
//     /* "source":0:101   */
//   stop
// Bytecode: 5f80355500
// Opcodes: PUSH0 DUP1 CALLDATALOAD SSTORE STOP
// SourceMappings: 67:9:0:-:0;13:15;;79:20;0:101
