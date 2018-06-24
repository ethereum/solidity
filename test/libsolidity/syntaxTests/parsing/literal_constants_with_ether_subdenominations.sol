contract c {
    function f()
    {
         a = 1 wei;
         b = 2 szabo;
         c = 3 finney;
         b = 4 ether;
    }
    uint256 a;
    uint256 b;
    uint256 c;
    uint256 d;
}
// ----
// Warning: (163-172): This declaration shadows an existing declaration.
// Warning: (17-128): No visibility specified. Defaulting to "public". 
