contract C {
    function f() public returns (bool ret) {
        return this.f > this.f;
    }
}
// ----
// TypeError: (73-88): Operator > not compatible with types function () external returns (bool) and function () external returns (bool)
