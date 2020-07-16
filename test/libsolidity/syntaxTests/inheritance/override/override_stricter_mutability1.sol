contract A {
	function foo() public payable virtual returns (uint256) {}
}
contract B is A {
	function foo() public override virtual returns (uint256) {}
}
// ----
// TypeError 6959: (94-153): Overriding function changes state mutability from "payable" to "nonpayable".
