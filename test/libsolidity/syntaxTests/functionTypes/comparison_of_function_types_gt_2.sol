contract C {
    function f() public returns (bool ret) {
        return f > f;
    }
}
// ----
// TypeError 2271: (73-78): Binary operator > not compatible with types function () returns (bool) and function () returns (bool).
