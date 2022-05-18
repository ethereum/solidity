abstract contract A {
	function foo() external virtual view returns(uint[] calldata);
}
contract X is A {
	function foo() public view override returns(uint[] memory) {  }
}
// ----
