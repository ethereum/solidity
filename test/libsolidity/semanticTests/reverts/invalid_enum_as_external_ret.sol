contract C {
    enum X {A, B}

    function test_return() public returns (X) {
        X garbled;
        assembly {
            garbled := 5
        }
        return garbled;
    }

    function test_inline_assignment() public returns (X _ret) {
        assembly {
            _ret := 5
        }
    }

    function test_assignment() public returns (X _ret) {
        X tmp;
        assembly {
            tmp := 5
        }
        _ret = tmp;
    }
}

// ====
// EVMVersion: >=byzantium
// compileToEwasm: also
// compileViaYul: also
// ----
// test_return() -> FAILURE, hex"4e487b71", 33 # both should throw #
// test_inline_assignment() -> FAILURE, hex"4e487b71", 33
// test_assignment() -> FAILURE, hex"4e487b71", 33
