object "a" {
    code {
        returncontract("b", 0, 0)
    }

    data "data1" "Hello, World!"
}

// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1
// ----
// TypeError 8970: (52-55): Unknown object "b".
