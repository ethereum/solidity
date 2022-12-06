struct S {
    uint v;
}

using {bitand as &} for S;

function bitand(S storage, S storage) returns (S storage) {
    S storage rTmp;
    return rTmp;
}

contract C {
    S s;
    function f() public {
        S storage sTmp;
        sTmp & s;
    }
}

// ----
// TypeError 3464: (145-149): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (234-238): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
