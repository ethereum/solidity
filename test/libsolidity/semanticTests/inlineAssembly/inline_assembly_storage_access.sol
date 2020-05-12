contract C {
    uint16 x;
    uint16 public y;
    uint256 public z;

    function f() public returns (bool) {
        uint256 off1;
        uint256 off2;
        assembly {
            sstore(z_slot, 7)
            off1 := z_offset
            off2 := y_offset
        }
        assert(off1 == 0);
        assert(off2 == 2);
        return true;
    }
}
// ====
// compileViaYul: also
// ----
// f() -> true
// z() -> 7
