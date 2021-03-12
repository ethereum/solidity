contract C {
    function f() public pure returns (function(uint) pure external returns (uint) g) {
        return g;
    }
}
// ----
