contract C
{
    function f() public pure returns (uint)
    {
        return;
    }
    function g() public pure returns (uint)
    {
        return (1, 2);
    }
}
// ----
// TypeError 6777: (71-78): Return arguments required.
// TypeError 5132: (143-156): Different number of arguments in return statement than in returns declaration.
