contract A {
    uint[] x;
}

contract B is A {
    function f() public view {
        A.x.length = 2;
    }
    function g() public pure returns (uint) {
        return A.x.length;
    }
    function h() public pure returns (uint) {
        return A.x[2];
    }
}
// ----
// TypeError: (87-97): Function declared as view, but this expression (potentially) modifies the state and thus requires non-payable (the default) or payable.
// TypeError: (170-173): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError: (170-180): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError: (249-252): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError: (249-255): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
