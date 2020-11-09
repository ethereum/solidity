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
//     ;; The Keccak-256 hash of the text representation of "B": 1eeffe5bc8d8819350ead60cc71ccd92c223cf52a908330db53461eb9ac89b62
//     ;; (@custom "B" "0061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e0000007b01430061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e0000003c01440061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a080106000240000b0b0a0901070002401a1a0b0b003c01450061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a080106000240000b0b0a0b01090002401a1a1a1a0b0b")
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
// 0061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e000000fa0101420061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e0000007b01430061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e0000003c01440061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a080106000240000b0b0a0901070002401a1a0b0b003c01450061736d01000000010401600000020100030201000503010001060100071102066d656d6f72790200046d61696e00000a080106000240000b0b0a0b01090002401a1a1a1a0b0b001305646174613148656c6c6f2c20576f726c64210a0901070002401a1a0b0b
