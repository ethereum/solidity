contract c {
    function c ()
    {
         a = 1 wei * 100 wei + 7 szabo - 3;
    }
    uint256 a;
}
// ----
// Warning: (17-86): Defining constructors as functions with the same name as the contract is deprecated. Use "constructor(...) { ... }" instead.
// Warning: (17-86): No visibility specified. Defaulting to "public". 
