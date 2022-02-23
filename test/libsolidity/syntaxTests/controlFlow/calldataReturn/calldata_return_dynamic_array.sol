contract C {
    function f() internal returns (uint256[] calldata) {}
}
// ----
// TypeError 3464: (48-66): This variable is of calldata pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
