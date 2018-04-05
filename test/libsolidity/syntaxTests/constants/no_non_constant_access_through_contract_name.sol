contract A {
    uint a = 0x1;
}
contract C {
    function f() public pure returns (uint) {
        return A.a;
    }
}
// ----
// TypeError: (107-110): Member "a" not found or not visible after argument-dependent lookup in type(contract A)
