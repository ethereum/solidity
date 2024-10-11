object "a" {
  code { sstore(0, datasize("data1")) }
  data "data1" "Hello, World!"
}
// ====
// EVMVersion: >=shanghai
// ----
// Assembly:
//     /* "source":44:61   */
//   0x0d
//     /* "source":41:42   */
//   0x00
//     /* "source":34:62   */
//   sstore
//     /* "source":22:70   */
//   stop
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// Bytecode: 600d5f5500fe
// Opcodes: PUSH1 0xD PUSH0 SSTORE STOP INVALID
// SourceMappings: 44:17:0:-:0;41:1;34:28;22:48
