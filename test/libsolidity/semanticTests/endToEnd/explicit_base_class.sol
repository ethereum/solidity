contract BaseBase {
    function g() public virtual returns(uint r) {
        return 1;
    }
}
contract Base is BaseBase {
    function g() public virtual override returns(uint r) {
        return 2;
    }
}
contract Derived is Base {
    function f() public returns(uint r) {
        return BaseBase.g();
    }

    function g() public override returns(uint r) {
        return 3;
    }
}

// ----
// g() -> 3
// g():"" -> "3"
// f() -> 1
// f():"" -> "1"
