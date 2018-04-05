contract A {
    uint constant c = 0;
}
contract B is A {
    function f() public pure returns (uint) {
        return super.c;
    }
}
// ----
// TypeError: (119-126): Member "c" not found or not visible after argument-dependent lookup in contract super B
