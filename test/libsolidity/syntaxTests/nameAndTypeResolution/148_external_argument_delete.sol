contract c {
    function f(uint a) external { delete a; }
}
// ----
// Warning: (17-58): Function state mutability can be restricted to pure
