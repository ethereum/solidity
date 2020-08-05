contract BaseBase {
    function g() public virtual returns (uint256 r) {
        return 1;
    }
}


contract Base is BaseBase {
    function g() public virtual override returns (uint256 r) {
        return 2;
    }
}


contract Derived is Base {
    function f() public returns (uint256 r) {
        return BaseBase.g();
    }

    function g() public override returns (uint256 r) {
        return 3;
    }
}
// ====
// compileViaYul: also
// ----
// g() -> 3
// f() -> 1
