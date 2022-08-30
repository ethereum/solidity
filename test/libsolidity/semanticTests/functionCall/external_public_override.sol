contract A {
    function f() external virtual returns (uint256) {
        return 1;
    }
}


contract B is A {
    function f() public override returns (uint256) {
        return 2;
    }

    function g() public returns (uint256) {
        return f();
    }
}

// ====
// compileToEwasm: also
// ----
// f() -> 2
// g() -> 2
