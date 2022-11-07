contract test {
    function f() public { int32(2) == uint64(2); }
}
// ----
// TypeError 2271: (42-63): Built-in binary operator == cannot be applied to types int32 and uint64.
