contract C {
    uint x;
    function g() public returns (uint) { return x; }
}
// ----
// Warning: (29-77): Function state mutability can be restricted to view
