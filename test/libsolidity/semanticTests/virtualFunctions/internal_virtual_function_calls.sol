contract Base {
    function f() public returns (uint256 i) {
        return g();
    }

    function g() internal virtual returns (uint256 i) {
        return 1;
    }
}


contract Derived is Base {
    function g() internal override returns (uint256 i) {
        return 2;
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 2
