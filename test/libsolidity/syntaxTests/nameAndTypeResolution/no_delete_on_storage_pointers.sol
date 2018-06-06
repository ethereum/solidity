contract C {
    uint[] data;
    function f() public {
        var x = data;
        delete x;
    }
}
// ----
// Warning: (64-69): Use of the "var" keyword is deprecated.
// TypeError: (86-94): Unary operator delete cannot be applied to type uint256[] storage pointer
