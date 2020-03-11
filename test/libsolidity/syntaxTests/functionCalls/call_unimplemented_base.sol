abstract contract I
{
    function a() internal view virtual returns(uint256);
}
abstract contract V is I
{
    function b() public view returns(uint256) { return a(); }
}
