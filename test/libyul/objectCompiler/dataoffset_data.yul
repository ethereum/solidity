object "a" {
  code { sstore(0, dataoffset("data1")) }
  data "data1" "Hello, World!"
}
// ----
// Assembly:
//     /* "source":32:51   */
//   data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f
//     /* "source":29:30   */
//   0x00
//     /* "source":22:52   */
//   sstore
//     /* "source":20:54   */
//   stop
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// Bytecode: 60065f5500fe48656c6c6f2c20576f726c6421
// Opcodes: PUSH1 0x6 PUSH0 SSTORE STOP INVALID BASEFEE PUSH6 0x6C6C6F2C2057 PUSH16 0x726C6421000000000000000000000000
// SourceMappings: 32:19:0:-:0;29:1;22:30;20:34
