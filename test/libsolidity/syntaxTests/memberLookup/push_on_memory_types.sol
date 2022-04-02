contract Test {
	function f() public pure {
        uint[] memory x;
        x.push(1);
	}
}
// ----
// TypeError 4994: (77-83='x.push'): Member "push" is not available in uint256[] memory outside of storage.
