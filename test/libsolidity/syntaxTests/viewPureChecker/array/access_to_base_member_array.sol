contract A {
    uint[] x;
}

contract B is A {
    function g() public pure returns (uint) {
        return A.x.length;
    }
    function h() public pure returns (uint) {
        return A.x[2];
    }
}
// ----
// TypeError 2527: (109-112): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (109-119): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (188-191): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (188-194): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
