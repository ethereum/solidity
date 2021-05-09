{
    function main() {
        i64.drop(i64.add(0, 1))
    }
}
// ====
// wasm: true
// ----
// Text:
// (module
//     (memory $memory (export "memory") 1)
//     (export "main" (func $main))
//
// (func $main
//     (block $label_
//         (drop (i64.add (i64.const 0) (i64.const 1)))
//     )
// )
//
// )
//
// Binary:
// 0061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a0d010b000240420042017c1a0b0b
