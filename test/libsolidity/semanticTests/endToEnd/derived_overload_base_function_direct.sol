contract B {
    function f() public returns(uint) {
        return 10;
    }
}
contract C is B {
    function f(uint i) public returns(uint) {
        return 2 * i;
    }

    function g() public returns(uint) {
        return f(1);
    }
}

// ====
// compileViaYul: also
// ----
// g() -> 2
