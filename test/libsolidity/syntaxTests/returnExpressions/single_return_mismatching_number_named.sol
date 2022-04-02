contract C
{
    function f() public pure returns (uint a)
    {
        return;
    }
    function g() public pure returns (uint a)
    {
        return (1, 2);
    }
}
// ----
// TypeError 6777: (73-80='return;'): Return arguments required.
// TypeError 5132: (147-160='return (1, 2)'): Different number of arguments in return statement than in returns declaration.
