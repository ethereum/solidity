contract C {
    enum X {A, B}
    X public x;

    function test_store() public returns (uint256) {
        X garbled = X.A;
        assembly {
            garbled := 5
        }
        x = garbled;
        return 1;
    }

    function test_store_ok() public returns (uint256) {
        x = X.A;
        return 1;
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// test_store_ok() -> 1
// x() -> 0
// test_store() -> FAILURE, hex"4e487b71", 33 # should throw #
