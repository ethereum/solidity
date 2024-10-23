object "a" {
    code {
        mstore(0, eofcreate("b", 0, 0, 0, 0))
        return(0, 32)
    }
}
// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1
// ----
// TypeError 8970: (52-55): Unknown object "b".
