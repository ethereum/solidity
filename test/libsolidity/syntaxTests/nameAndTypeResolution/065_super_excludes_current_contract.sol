contract A {
    function b() public {}
}

contract B is A {
    function f() public {
        super.f();
    }
}
// ----
// TypeError 9582: (95-102): Member "f" not found or not visible after argument-dependent lookup in contract super B.
