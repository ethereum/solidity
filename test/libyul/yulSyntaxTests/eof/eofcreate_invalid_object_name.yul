object "a" {
    code {
        mstore(0, eofcreate("a.b", 0, 0, 0, 0))
        return(0, 32)
    }

    object "b" {
        code {}
    }
}
// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1
// ----
// TypeError 2186: (52-57): Name required but path given as "eofcreate" argument.
// TypeError 8970: (52-57): Unknown object "a.b".
