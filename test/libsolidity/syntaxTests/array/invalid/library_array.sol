library L {}
contract C {
	function f() public {
		L[] memory x;
	}
}
// ----
// TypeError 7486: (51-63): Arrays of libraries are not allowed.
