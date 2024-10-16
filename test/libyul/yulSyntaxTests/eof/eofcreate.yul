object "a" {
    code {
        let success := eofcreate("b", 0, 0, 0, 0)
        return(0, 0)
    }

    object "b" {
        code {
            returncontract("c", 0, 0)
        }
        object "c" {
            code {
            }
        }
    }

    data "data1" "Hello, World!"
}

// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1