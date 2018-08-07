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
// TypeError: (71-78): Return arguments required.
// TypeError: (143-156): Different number of arguments in return statement than in returns declaration.
