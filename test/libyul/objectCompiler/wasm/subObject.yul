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
//     ;; custom section for sub-module
//     ;; The Keccak-256 hash of the text representation of "sub": 78ac3419d75c8d6f42f663717b8e964eeb994d77ff175145133084422dbd23d7
//     ;; (@custom "sub" "0061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a0a0108000240420b1a0b0b")
//     ;; custom section for data
//     ;; (@custom "str" "48656c6c6f2c20576f726c6421")
//     (memory $memory (export "memory") 1)
//
// )
//
// Binary:
// 0061736d010000000101000201000301000503010001060100070a01066d656d6f727902000040037375620061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a0a0108000240420b1a0b0b00110373747248656c6c6f2c20576f726c64210a0100
