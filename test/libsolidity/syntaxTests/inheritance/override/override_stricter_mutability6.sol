contract A {
	function foo() public virtual returns (uint256) {}
}
contract B is A {
	function foo() public payable override virtual returns (uint256) {}
}
// ----
// TypeError 6959: (86-153): Overriding function changes state mutability from "nonpayable" to "payable".
