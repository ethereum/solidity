contract C {
    struct S { uint256 x; }
    function f() internal returns (S calldata) {}
}
// ----
// TypeError 3464: (76-86): This variable is of calldata pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
