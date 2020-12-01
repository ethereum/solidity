object "A" {
    code {
        function main() {
            // TODO: support this
            // i64.drop(dataoffset("A"))
            // i64.drop(datasize("A"))
            i64.drop(dataoffset("B"))
            i64.drop(datasize("B"))
            // TODO: support sub-subobjects
            // i64.drop(dataoffset("B.C"))
            // i64.drop(datasize("B.C"))
            // i64.drop(dataoffset("B.E"))
            // i64.drop(datasize("B.E"))
            // i64.drop(dataoffset("B.C.D"))
            // i64.drop(datasize("B.C.D"))
        }
    }

    data "data1" "Hello, World!"

    object "B" {
        code {
            function main() {
                i64.drop(dataoffset("C"))
                i64.drop(datasize("C"))
                i64.drop(dataoffset("E"))
                i64.drop(datasize("E"))
                // i64.drop(dataoffset("C.D"))
                // i64.drop(datasize("C.D"))
            }
        }
        object "C" {
            code {
                function main() {
                    i64.drop(dataoffset("D"))
                    i64.drop(datasize("D"))
                }
            }
            object "D" {
                code {
                    function main() {
                        unreachable()
                    }
                }
            }
        }
        object "E" {
            code {
                function main() {
                    unreachable()
                }
            }
        }
    }
}
// ====
// wasm: true
// ----
// Text:
// (module
//     ;; custom section for sub-module
//     ;; The Keccak-256 hash of the text representation of "B": ccfc48ce1c0d0542ffd25ae6858777b2f7b8a6d2b6608f679458182e719f5434
//     ;; (@custom "B" "0061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e0000007f01430061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e0000003c01440061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a080106000240000b0b0a0d010b00024042341a423a1a0b0b003c01450061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a080106000240000b0b0a15011300024042341a42fd001a42b5011a423a1a0b0b")
//     ;; custom section for data
//     ;; (@custom "data1" "48656c6c6f2c20576f726c6421")
//     (memory $memory (export "memory") 1)
//     (export "main" (func $main))
//
// (func $main
//     (block $label_
//         (drop (dataoffset "B"))
//         (drop (datasize "B"))
//     )
// )
//
// )
//
// Binary:
// 0061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e000000880201420061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e0000007f01430061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e0000003c01440061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a080106000240000b0b0a0d010b00024042341a423a1a0b0b003c01450061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a080106000240000b0b0a15011300024042341a42fd001a42b5011a423a1a0b0b001305646174613148656c6c6f2c20576f726c64210a0e010c00024042351a4286021a0b0b
