contract A {
    function f() virtual internal returns(uint) {
        return 1;
    }
}
contract B is A {
    function f() internal override returns(uint) {
        return 2;
    }

    function g() public returns(uint) {
        return A.f();
    }
}

// ----
// g() -> 1
// g():"" -> "1"
