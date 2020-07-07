==== Source: A ====
contract A {
	function g(uint256 x) public view returns(uint256) { return x; }
}
==== Source: B ====
import "A";
contract B is A {
	function f(uint256 x) public view returns(uint256) { return x; }
}
// ----
// Warning 2018: (A:14-78): Function state mutability can be restricted to pure
// Warning 2018: (B:31-95): Function state mutability can be restricted to pure
