contract C {
    function f() public returns (function(uint) external returns (uint) g) {
        return g;
    }
}
