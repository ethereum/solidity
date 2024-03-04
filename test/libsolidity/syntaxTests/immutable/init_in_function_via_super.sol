contract B {
    uint immutable x;

    function g() public {}
}

contract C is B {
    function f() public {
        super.x = 42;
    }
}
// ----
// TypeError 9582: (118-125): Member "x" not found or not visible after argument-dependent lookup in type(contract super C).
