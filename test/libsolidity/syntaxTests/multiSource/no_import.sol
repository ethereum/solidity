==== Source: A ====
contract A {
	function g(uint256 x) public view returns(uint256) { return x; }
}
==== Source: B ====
contract B is A {
	function f(uint256 x) public view returns(uint256) { return x; }
}
// ----
// DeclarationError: (B:14-15): Identifier not found or not unique.
