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
// TypeError 4259: (177-182): Invalid type for argument in the function call. An enum type, contract type or an integer type is required, but type(contract super SuperTest) provided.
