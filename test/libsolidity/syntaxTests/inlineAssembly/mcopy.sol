contract C {
    function f() public pure {
        assembly {
            mcopy(0, 0, 0)
            mcopy(0x1000, 0x2000, 100)
        }
    }
}
// ====
// EVMVersion: >=cancun
