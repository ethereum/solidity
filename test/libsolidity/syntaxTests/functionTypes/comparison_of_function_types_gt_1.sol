contract C {
    function f() public returns (bool ret) {
        return this.f > this.f;
    }
}
// ----
// TypeError 2271: (73-88): Binary operator > not compatible with types function () external returns (bool) and function () external returns (bool).
