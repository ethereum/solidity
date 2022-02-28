contract A {
    struct S { uint x; }
    S s;
}

contract B is A {
    function f() public view {
        A.s = A.S(2);
    }
    function g() public view {
        A.s.x = 2;
    }
    function h() public pure returns (uint) {
        return A.s.x;
    }
}
// ----
// TypeError 8961: (107-110): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 8961: (166-171): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 2527: (244-247): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (244-249): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
