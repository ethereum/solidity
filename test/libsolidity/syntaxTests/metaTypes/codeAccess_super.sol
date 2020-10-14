contract Other {
    function f(uint) public pure returns (uint) {}
}
contract SuperTest is Other {
	function creationSuper() public pure returns (bytes memory) {
		return type(super).creationCode;
	}
	function runtimeOther() public pure returns (bytes memory) {
		return type(super).runtimeCode;
	}
}
// ----
// TypeError 3625: (172-196): "creationCode" and "runtimeCode" are not available for the "super" contract.
// TypeError 3625: (272-295): "creationCode" and "runtimeCode" are not available for the "super" contract.
