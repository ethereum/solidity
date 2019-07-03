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
// TypeError: (107-110): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError: (157-160): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
