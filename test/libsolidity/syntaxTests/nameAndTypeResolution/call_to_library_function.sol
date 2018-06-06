library Lib {
    function min(uint, uint) public returns (uint);
}
contract Test {
    function f() public {
        uint t = Lib.min(12, 7);
    }
}
// ----
// Warning: (118-124): Unused local variable.
