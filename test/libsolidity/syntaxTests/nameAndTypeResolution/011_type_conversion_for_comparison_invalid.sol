contract test {
    function f() public { int32(2) == uint64(2); }
}
// ----
// TypeError: (42-63): Operator == not compatible with types int32 and uint64
