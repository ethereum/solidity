contract C
{
    function f() public pure returns (uint a, uint b)
    {
        return 1;
    }
    function g() public pure returns (uint a, uint b)
    {
        return (1, 2, 3);
    }
    function h() public pure returns (uint a, uint b)
    {
        return;
    }
}
// ----
// TypeError: (81-89): Different number of arguments in return statement than in returns declaration.
// TypeError: (165-181): Different number of arguments in return statement than in returns declaration.
// TypeError: (257-264): Return arguments required.
