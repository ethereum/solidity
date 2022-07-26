contract C {
    uint256[] s;
    function f() public view {
        uint256[] storage x;
        uint256[] storage y = (x = s)[0] > 0 ? x : x;
        y;
    }
}
// ---
// ----
// TypeError 3464: (137-138): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (141-142): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
