contract A {
    function f() public virtual returns (uint) {
        return 42;
    }
}

interface I {
    function f() external returns (uint);
}

contract B is A, I {
    function f() override(A, I) public returns (uint) {
        // I.f() is before A.f() in the C3 linearized order
        // but it has no implementation.
        return super.f();
    }
}
// ----
// f() -> 42
