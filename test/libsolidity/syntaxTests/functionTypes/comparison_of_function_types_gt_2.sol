contract C {
    function f() public returns (bool ret) {
        return f > f;
    }
}
// ----
// TypeError 2271: (73-78='f > f'): Operator > not compatible with types function () returns (bool) and function () returns (bool)
