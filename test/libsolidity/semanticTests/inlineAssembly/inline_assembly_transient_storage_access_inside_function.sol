contract C {
    uint16 transient x;
    uint16 public transient y;
    uint256 public transient z;

    function f() public returns (uint256) {
        uint256 offset;
        assembly {
            function f() -> o1 {
                tstore(z.slot, 7)
                o1 := y.offset
            }
            offset := f()
        }
        assert(offset == 2);
        return z;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f() -> 7
