contract C
{
    function f() public pure {
        return;
    }
    function g() public pure returns (uint) {
        return 1;
    }
    function h() public pure returns (uint a) {
        return 1;
    }
    function i() public pure returns (uint, uint) {
        return (1, 2);
    }
    function j() public pure returns (uint a, uint b) {
        return (1, 2);
    }
}
