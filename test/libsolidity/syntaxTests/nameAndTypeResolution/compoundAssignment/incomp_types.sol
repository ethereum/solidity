contract C {
    function f() public returns (uint a, uint b) {
        a += (1, 1);
    }
}
// ----
// TypeError: (72-83): Operator += not compatible with types uint256 and tuple(int_const 1,int_const 1)
