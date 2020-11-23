==== Source: A ====
contract A {
	function g(uint256 x) public view returns(uint256) { return x + 1; }
}
==== Source: B ====
import "A";
contract B is A {
	function f(uint256 x) public view returns(uint256) { return x; }
}
// ====
// compileViaYul: also
// compileToEwasm: also
// ----
// f(uint256): 1337 -> 1337
// g(uint256): 1337 -> 1338
