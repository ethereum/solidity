contract A {
    function f() public virtual returns(uint r) {
        return 1;
    }
}
contract B is A {
    function f() public virtual override returns(uint r) {
        return super.f() | 2;
    }
}
contract C is A {
    function f() public virtual override returns(uint r) {
        return super.f() | 4;
    }
}
contract D is B, C {
    function f() public override(B, C) returns(uint r) {
        return super.f() | 8;
    }
}

// ----
// f() -> 15
