contract A {
    function f() private {}
}

contract B {
    function g() external {
        A.f;
    }
}
// ----
// TypeError 9582: (93-96='A.f'): Member "f" not found or not visible after argument-dependent lookup in type(contract A).
