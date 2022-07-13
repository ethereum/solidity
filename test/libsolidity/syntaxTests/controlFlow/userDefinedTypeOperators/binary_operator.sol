struct S { bool f; }

using {add as +} for S;

function add(S storage x, S storage) pure returns (S storage) { return x; }

contract C {
    S s;
    function f() public {
        S storage x = s;
        S storage y;
        x + y;
    }
}

// ----
// TypeError 3464: (230-231): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
