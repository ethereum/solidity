abstract contract I
{
    function a() internal view virtual returns(uint256);
}
abstract contract V is I
{
    function b() public view returns(uint256) { return a(); }
}
contract C is V
{
    function a() internal view override returns (uint256) { return 42;}
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// b() -> 42
