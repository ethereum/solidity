struct S {
    uint256 v;
}

library L {
    function double(S memory s) pure public returns(uint256) {
        return 2 * s.v;
    }
}

using L for S;

contract C
{
    S private s;
    function test() pure private {
        s.double;
    }
}
// ----
// TypeError 2527: (226-227): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
// TypeError 2527: (226-234): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
