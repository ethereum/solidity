object "a" {
  code { sstore(0, datasize("data1")) }
  data "data1" "Hello, World!"
}
// ----
// Assembly:
//     /* "source":32:49   */
//   0x0d
//     /* "source":29:30   */
//   0x00
//     /* "source":22:50   */
//   sstore
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// Bytecode: 600d5f55fe
// Opcodes: PUSH1 0xD PUSH0 SSTORE INVALID
// SourceMappings: 32:17:0:-:0;29:1;22:28
