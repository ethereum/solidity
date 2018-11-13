contract test {
    function f(function(uint) external returns (uint) g) internal returns (uint a) {
        return g(1);
    }
}
