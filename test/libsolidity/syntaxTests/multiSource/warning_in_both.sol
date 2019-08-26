==== Source: A ====
contract A {
	function g(uint256 x) public view returns(uint256) { return x; }
}
==== Source: B ====
contract B {
	function f(uint256 x) public view returns(uint256) { return x; }
}
// ----
// Warning: (A:14-78): Function state mutability can be restricted to pure
// Warning: (B:14-78): Function state mutability can be restricted to pure
