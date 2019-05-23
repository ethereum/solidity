contract C {
    function f() public returns (uint) {
        return 0;
    }
    function f(uint a) public returns (uint) {
        return a;
    }
    function f(uint a, uint b) public returns (uint) {
        return a+b;
    }
    function f(uint a, uint b, uint c) public returns (uint) {
        return a+b+c;
    }
    function call(uint num) public returns (uint256) {
        if (num == 0)
            return f();
        if (num == 1)
            return f({a: 1});
        if (num == 2)
            return f({b: 1, a: 2});
        if (num == 3)
            return f({c: 1, a: 2, b: 3});

        return 500;
    }
}
// ====
// compileViaYul: also
// ----
// call(uint256): 0 -> 0
// call(uint256): 1 -> 1
// call(uint256): 2 -> 3
// call(uint256): 3 -> 6
// call(uint256): 4 -> 500
