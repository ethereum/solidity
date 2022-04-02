contract test {
    function f() public pure {
        int x;
        uint y = uint(-x);
        -y;
    }
}
// ----
// TypeError 4907: (97-99='-y'): Unary operator - cannot be applied to type uint256
