contract A {
    function f(uint256 a) public returns (uint256) {
        return 2 * a;
    }
}


contract B {
    function f() public returns (uint256) {
        return 10;
    }
}


contract C is A, B {
    function g() public returns (uint256) {
        return f();
    }

    function h() public returns (uint256) {
        return f(1);
    }
}

// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// g() -> 10
// h() -> 2
