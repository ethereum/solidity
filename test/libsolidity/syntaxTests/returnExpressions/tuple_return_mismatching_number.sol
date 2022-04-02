contract C
{
    function f() public pure returns (uint, uint)
    {
        return 1;
    }
    function g() public pure returns (uint, uint)
    {
        return (1, 2, 3);
    }
    function h() public pure returns (uint, uint)
    {
        return;
    }
}
// ----
// TypeError 8863: (77-85='return 1'): Different number of arguments in return statement than in returns declaration.
// TypeError 5132: (157-173='return (1, 2, 3)'): Different number of arguments in return statement than in returns declaration.
// TypeError 6777: (245-252='return;'): Return arguments required.
