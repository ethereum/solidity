contract test {
    function f() public pure {
        int x;
        uint y = uint(-x);
        -y;
    }
}
// ----
// TypeError 4907: (97-99): Unary operator - cannot be applied to type uint256. Unary negation is only allowed for signed integers.
