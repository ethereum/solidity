contract A {
    uint constant x = 2;
}

contract B is A {
    function f() public pure returns (uint) {
        return A.x;
    }
}
// ----
