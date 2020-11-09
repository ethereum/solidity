object "a" {
    code {}
    // Unreferenced data is not added to the assembled bytecode.
    data "str" "Hello, World!"
    object "sub" { code { function main() { i64.drop(11) } } }
}
// ====
// wasm: true
// ----
// Text:
// (module
//     ;; sub-module "sub" will be encoded as custom section in binary here, but is skipped in text mode.
//     ;; custom-section "str" will be encoded as custom section in binary here, but is skipped in text mode.
//     (memory $memory (export "memory") 1)
//
// )
//
// Binary:
// 0061736d010000000101000201000301000503010001060100070a01066d656d6f72790200003e037375620061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a0801060002401a0b0b00110373747248656c6c6f2c20576f726c64210a0100
