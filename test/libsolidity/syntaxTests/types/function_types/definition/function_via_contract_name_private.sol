contract A {
    function f() private {}
}

contract B {
    function g() external {
        A.f;
    }
}
// ----
// TypeError: (93-96): Member "f" not found or not visible after argument-dependent lookup in type(contract A).
