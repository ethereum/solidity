contract C {
    enum X {A, B}

    function tested(X x) public returns (uint256) {
        return 1;
    }

    function test() public returns (uint256) {
        X garbled;

        assembly {
            garbled := 5
        }

        return this.tested(garbled);
    }
}
// ====
// EVMVersion: >=byzantium
// compileToEwasm: also
// ----
// test() -> FAILURE, hex"4e487b71", 0x21 # should throw #
