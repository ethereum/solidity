contract test {
    function f() public { uint32(2) == uint64(2); }
}
// ----
// Warning 6133: (42-64): Statement has no effect.
// Warning 2018: (20-67): Function state mutability can be restricted to pure
