contract A {
    function f() internal {}
}

contract B {
    function g() external {
        A.f;
    }
}
// ----
// TypeError 9582: (94-97): Member "f" not found or not visible after argument-dependent lookup in type(contract A).
