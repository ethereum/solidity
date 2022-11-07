contract C {
    function f() public returns (bool ret) {
        return this.f < this.f;
    }
}
// ----
// TypeError 2271: (73-88): Built-in binary operator < cannot be applied to types function () external returns (bool) and function () external returns (bool).
