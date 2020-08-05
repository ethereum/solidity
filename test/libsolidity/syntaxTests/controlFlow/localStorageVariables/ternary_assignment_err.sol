contract C {
    uint256[] s;
    function f() public {
        bool d;
        uint256[] storage x;
        uint256[] storage y = d ? (x = s) : x;
        y;
    }
}
// ----
// TypeError 3464: (145-146): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
