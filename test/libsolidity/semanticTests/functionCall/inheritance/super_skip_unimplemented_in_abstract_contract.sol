contract A {
    function f() public virtual returns (uint) {
        return 42;
    }
}

abstract contract I {
    function f() external virtual returns (uint);
}

contract B is A, I {
    function f() override(A, I) public returns (uint) {
        // I.f() is before A.f() in the C3 linearized order
        // but it has no implementation.
        return super.f();
    }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// f() -> 42
