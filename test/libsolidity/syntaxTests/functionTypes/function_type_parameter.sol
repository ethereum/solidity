contract C {
    function f(function(uint) external returns (uint) g) public returns (function(uint) external returns (uint)) {
        return g;
    }
}
