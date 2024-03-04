contract C {
    function f() public pure {
        assembly {
            mcopy(1, 2, 3)
        }
    }
}
// ====
// EVMVersion: >=cancun
