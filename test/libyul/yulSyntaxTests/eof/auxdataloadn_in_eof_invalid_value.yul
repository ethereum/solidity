object "a" {
    code {
        mstore(0, auxdataloadn(0x01FFFF))
        return(0, 32)
    }

    data "data1" "Hello, World!"
}

// ====
// EVMVersion: >=shanghai
// bytecodeFormat: >=EOFv1
// ----
// TypeError 5202: (55-63): Invalid auxdataloadn argument value. Offset must be in range 0...0xFFFF
