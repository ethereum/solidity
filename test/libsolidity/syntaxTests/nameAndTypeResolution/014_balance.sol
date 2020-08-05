contract test {
    function fun() public {
        uint256 x = address(0).balance;
    }
}
// ----
// Warning 2072: (52-61): Unused local variable.
// Warning 2018: (20-89): Function state mutability can be restricted to view
