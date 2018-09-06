contract test {
    function f() public { uint32(2) == int64(2); }
}
// ----
// Warning: (42-63): Statement has no effect.
// Warning: (20-66): Function state mutability can be restricted to pure
