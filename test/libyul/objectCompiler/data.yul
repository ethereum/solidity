object "a" {
  code {}
  // Unreferenced data is not added to the assembled bytecode.
  data "str" "Hello, World!"
}
// ----
// Assembly:
//     /* "source":20:22   */
//   stop
// stop
// data_acaf3289d7b601cbd114fb36c4d29c85bbfd5e133f14cb355c3fd8d99367964f 48656c6c6f2c20576f726c6421
// Bytecode: 00fe
// Opcodes: STOP INVALID
// SourceMappings: 20:2:0:-:0
