object "a" {
    code {
        mstore(0, auxdataloadn("0"))
        return(0, 32)
    }

    data "data1" "Hello, World!"
    }

// ====
// EVMVersion: >=prague
// bytecodeFormat: >=EOFv1
// ----
// TypeError 5859: (55-58): Function expects number literal.