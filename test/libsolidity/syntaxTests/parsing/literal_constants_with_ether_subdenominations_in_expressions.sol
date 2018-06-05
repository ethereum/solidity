contract c {
    constructor()
    {
         a = 1 wei * 100 wei + 7 szabo - 3;
    }
    uint256 a;
}
// ----
// Warning: (17-86): No visibility specified. Defaulting to "public". 
