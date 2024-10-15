object "a" {
    code {
        mstore(0, auxdataloadn(0))
        return(0, 32)
    }

    data "data1" "Hello, World!"
}

// ====
// EVMVersion: >=prague
// bytecodeFormat: legacy
// ----
// DeclarationError 4619: (42-54): Function "auxdataloadn" not found.
// TypeError 3950: (42-57): Expected expression to evaluate to one value, but got 0 values instead.