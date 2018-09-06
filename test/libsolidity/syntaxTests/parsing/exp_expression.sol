contract test {
    function fun(uint256 a) public {
        uint256 x = 3 ** a;
    }
}
// ----
// Warning: (61-70): Unused local variable.
// Warning: (20-86): Function state mutability can be restricted to pure
