contract A {
    function f(uint256 a) public returns (uint256) {
        return 2 * a;
    }
}


contract B {
    function f(bool b) public returns (uint256) {
        return 10;
    }
}


contract C is A, B {
    function g() public returns (uint256) {
        return super.f(true);
    }

    function h() public returns (uint256) {
        return super.f(1);
    }
}

// ====
// compileToEwasm: also
// ----
// g() -> 10
// h() -> 2
