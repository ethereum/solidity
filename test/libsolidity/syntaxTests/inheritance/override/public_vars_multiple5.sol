contract ERC20 {
  function balanceOf(address, uint) external virtual view returns (uint) {}
  function balanceOf(uint) external virtual view returns (uint) {}
  function balanceOf() external virtual view returns (uint) {}
}
contract C is ERC20 {
  mapping(address => uint) public override balanceOf;
}
// ----
// TypeError 7792: (281-289): Public state variable has override specified but does not override anything.
