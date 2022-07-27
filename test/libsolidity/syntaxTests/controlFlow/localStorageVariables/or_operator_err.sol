struct S {
    uint v;
}

using {bitor as |} for S;

function bitor(S storage, S storage) returns (S storage) {
    S storage rTmp;
    return rTmp;
}

contract C {
    S s;
    function f() public {
        S storage sTmp;
        sTmp | s;
    }
}

// ----
// TypeError 3464: (143-147): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (232-236): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
