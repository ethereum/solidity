library L {}
contract C {
	function f() public override (L) {}
}
// ----
// TypeError 1130: (57-58): Invalid use of a library name.
