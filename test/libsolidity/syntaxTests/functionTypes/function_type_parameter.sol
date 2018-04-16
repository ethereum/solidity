contract C {
    uint x;
    function f(function(uint) external returns (uint) g) public returns (function(uint) external returns (uint)) {
        x = 2;
        return g;
    }
}
