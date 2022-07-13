struct S { int f; }

using {sub as -} for S;

function sub(S storage x) pure returns (S storage) { return x; }

contract C {
    function f() public {
        S storage y;
        -y;
    }
}

// ----
// TypeError 3464: (181-182): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
