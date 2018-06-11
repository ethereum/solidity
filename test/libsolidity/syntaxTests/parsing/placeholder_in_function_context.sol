contract c {
    function fun() returns (uint r) {
        uint _ = 8;
        return _ + 1;
    }
}
// ----
// Warning: (17-98): No visibility specified. Defaulting to "public". 
// Warning: (17-98): Function state mutability can be restricted to pure
