contract A {
    uint constant a = 0x1;
}
contract C {
    function f() public pure returns (uint) {
        return A.a;
    }
}
