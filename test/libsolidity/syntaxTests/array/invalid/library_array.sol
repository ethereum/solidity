library L {}
contract C {
	function f() public {
		L[] memory x;
	}
}
// ----
// TypeError 1130: (51-52): Invalid use of a library name.
