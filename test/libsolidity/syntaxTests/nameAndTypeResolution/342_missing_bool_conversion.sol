contract test {
    function b(uint a) public {
        bool(a == 1);
    }
}
// ----
// Warning 2018: (20-75): Function state mutability can be restricted to pure
