contract c {
    function fun() public returns (uint r) {
        uint _ = 8;
        return _ + 1;
    }
}
// ----
// Warning: (17-105): Function state mutability can be restricted to pure
