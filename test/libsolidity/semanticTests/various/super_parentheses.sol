contract A {
    function f() public virtual returns (uint256 r) {
        return 1;
    }
}


contract B is A {
    function f() public virtual override returns (uint256 r) {
        return ((super).f)() | 2;
    }
}


contract C is A {
    function f() public virtual override returns (uint256 r) {
        return ((super).f)() | 4;
    }
}


contract D is B, C {
    function f() public override(B, C) returns (uint256 r) {
        return ((super).f)() | 8;
    }
}

// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f() -> 15
