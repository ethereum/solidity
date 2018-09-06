contract test {
    function fun() public pure {
        uint256 x;
        while (true) { x = 1; break; continue; } x = 9;
    }
}
// ----
