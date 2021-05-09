contract ERC20 {
  function balanceOf(address) external virtual view returns (uint) {}
  function balanceOf(uint) external virtual view returns (uint) {}
  function balanceOf() external virtual view returns (uint) {}
}
contract C is ERC20 {
  mapping(address => uint) public override balanceOf;
}
// ----
