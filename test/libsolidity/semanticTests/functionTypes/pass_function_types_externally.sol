contract C {
    function f(uint256 x) public returns (uint256) {
        return this.eval(this.g, x);
    }

    function f2(uint256 x) public returns (uint256) {
        return eval(this.g, x);
    }

    function eval(function(uint) external returns (uint) x, uint a) public returns (uint) {
        return x(a);
    }

    function g(uint256 x) public returns (uint256) {
        return x + 1;
    }
}

// ====
// compileViaYul: also
// ----
// f(uint256): 7 -> 8
// f2(uint256): 7 -> 8
