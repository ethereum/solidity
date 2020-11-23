contract C {
    enum X {A, B}

    function test_eq() public returns (bool) {
        X garbled;
        assembly {
            garbled := 5
        }
        return garbled == garbled;
    }

    function test_eq_ok() public returns (bool) {
        X garbled = X.A;
        return garbled == garbled;
    }

    function test_neq() public returns (bool) {
        X garbled;
        assembly {
            garbled := 5
        }
        return garbled != garbled;
    }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// EVMVersion: >=byzantium
// ----
// test_eq_ok() -> 1
// test_eq() -> FAILURE, hex"4e487b71", 33 # both should throw #
// test_neq() -> FAILURE, hex"4e487b71", 33
