contract test {
    function fun(uint256 a) returns (address b) {
        if (a < 0) b = 0x67; else if (a == 0) b = 0x12; else b = 0x78;
    }
}
// ----
// Warning: (20-142): No visibility specified. Defaulting to "public". 
// Warning: (20-142): Function state mutability can be restricted to pure
