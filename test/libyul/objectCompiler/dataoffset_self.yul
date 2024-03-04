object "a" {
  code { sstore(0, dataoffset("a")) }
  data "data1" "Hello, World!"
}
// ----
// Assembly:
//     /* "source":32:47   */
//   0x00
//     /* "source":29:30   */
//   dup1
//     /* "source":22:48   */
//   sstore
//     /* "source":20:50   */
//   stop
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// Bytecode: 5f805500fe
// Opcodes: PUSH0 DUP1 SSTORE STOP INVALID
// SourceMappings: 32:15:0:-:0;29:1;22:26;20:30
