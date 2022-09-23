contract C {
    function f() public returns (bool ret) {
        return f < f;
    }
}
// ----
// TypeError 2271: (73-78): Built-in binary operator < cannot be applied to types function () returns (bool) and function () returns (bool).
