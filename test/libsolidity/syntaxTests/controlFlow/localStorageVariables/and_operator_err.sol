struct S { bool f; }

contract C {
    function f() public view {
        S storage s;
        s.f && true;
    }
}

// ----
// TypeError 3464: (95-96): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
