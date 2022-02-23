abstract contract I
{
    function a() internal view virtual returns(uint256);
}
abstract contract J is I
{
    function a() internal view virtual override returns(uint256);
}
abstract contract V is J
{
    function b() public view returns(uint256) { return a(); }
}
contract C is V
{
    function a() internal view override returns (uint256) { return 42; }
}
// ====
// compileToEwasm: also
// compileViaYul: also
// ----
// b() -> 42
