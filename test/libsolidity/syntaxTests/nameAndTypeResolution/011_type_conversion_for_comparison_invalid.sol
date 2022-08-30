contract test {
    function f() public { int32(2) == uint64(2); }
}
// ----
// TypeError 2271: (42-63): Binary operator == not compatible with types int32 and uint64.
