contract A {
    uint x;
}

contract B is A {
    function f() public pure returns (uint) {
        return A.x;
    }
    function g() public view {
        A.x = 5;
    }
}
// ----
// TypeError 2527: (107-110): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 8961: (157-160): Function cannot be declared as view because this expression (potentially) modifies the state.
