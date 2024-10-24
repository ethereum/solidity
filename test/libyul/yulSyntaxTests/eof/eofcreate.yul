object "a" {
    code {
        let addr := eofcreate("b", 0, 0, 0, 0)
        return(0, 0)
    }

    object "b" {
        code {}
    }
}
// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1
