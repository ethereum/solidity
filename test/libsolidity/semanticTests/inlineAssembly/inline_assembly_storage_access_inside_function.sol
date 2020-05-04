contract C {
    uint16 x;
    uint16 public y;
    uint256 public z;

    function f() public returns (bool) {
        uint256 off1;
        uint256 off2;
        assembly {
            function f() -> o1 {
                sstore(z_slot, 7)
                o1 := y_offset
            }
            off2 := f()
        }
        assert(off2 == 2);
        return true;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> true
// z() -> 7
