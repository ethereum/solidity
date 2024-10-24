object "a" {
    code {
        returncontract("b", 0, 0)
    }
}
// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1
// ----
// TypeError 8970: (47-50): Unknown object "b".
