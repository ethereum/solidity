{
  let x := calldataload(0)
  let y := calldataload(0)
  let z := sub(y, x)
  sstore(add(x, 0), z)
}
// ====
// EVMVersion: >=shanghai
// optimizationPreset: full
// ----
// Assembly:
//     /* "source":63:64   */
//   0x00
//     /* "source":46:61   */
//   dup1
//   calldataload
//     /* "source":39:65   */
//   sstore
//     /* "source":27:73   */
//   stop
// Bytecode: 5f80355500
// Opcodes: PUSH0 DUP1 CALLDATALOAD SSTORE STOP
// SourceMappings: 63:1:0:-:0;46:15;;39:26;27:46
