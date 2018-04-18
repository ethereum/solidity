contract Test {
	function f() public pure {
        uint[] memory x;
        x.push(1);
	}
}
// ----
// TypeError: (77-83): Member "push" is not available in uint256[] memory outside of storage.
