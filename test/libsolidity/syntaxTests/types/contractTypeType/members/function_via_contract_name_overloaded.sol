contract A {
    function f() external {}
    function f(uint256) external {}
}

contract B {
    function g() external {
        A.f;
    }
}
// ----
// TypeError 6675: (130-133): Member "f" not unique after argument-dependent lookup in type(contract A).
