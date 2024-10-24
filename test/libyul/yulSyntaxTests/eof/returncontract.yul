object "b" {
    code {
        returncontract("c", 0, 0)
    }

    object "c" {
        code {}
    }
}
// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1
