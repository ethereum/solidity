// something else
/*-- another unrelated comment --*/
/// @use-src 1: "abc.sol" , 0: "def.sol"
object "a" {
  code {
    /// @src 1:0:2
    datacopy(0, dataoffset("sub"), datasize("sub"))
    return(0,
      /** @src 0:5:6 */
      datasize("sub")
    )
  }
  /// @use-src 1: "abc.sol" , 0: "def.sol"
  object "sub" {
    code {
      /// @src 0:70:72
      sstore(0, dataoffset("sub"))
      /**
       * @something else
       * @src 1:2:5
       */
      mstore(
        0,
        datasize("data1")
        /// @src 1:90:2
      )
    }
    data "data1" "Hello, World!"
  }
}
// ====
// EVMVersion: >=shanghai
// ----
// Assembly:
//     /* "abc.sol":0:2   */
//   codecopy(0x00, dataOffset(sub_0), dataSize(sub_0))
//     /* "def.sol":5:6   */
//   dataSize(sub_0)
//     /* "abc.sol":0:2   */
//   0x00
//   return
// stop
//
// sub_0: assembly {
//         /* "def.sol":70:72   */
//       sstore(0x00, 0x00)
//         /* "abc.sol":2:5   */
//       mstore(0x00, 0x0d)
//       stop
//     stop
//     data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// }
// Bytecode: 6009600b5f3960095ff3fe5f5f55600d5f5200fe
// Opcodes: PUSH1 0x9 PUSH1 0xB PUSH0 CODECOPY PUSH1 0x9 PUSH0 RETURN INVALID PUSH0 PUSH0 SSTORE PUSH1 0xD PUSH0 MSTORE STOP INVALID
// SourceMappings: 0:2::-:0;;;;5:1;0:2;
