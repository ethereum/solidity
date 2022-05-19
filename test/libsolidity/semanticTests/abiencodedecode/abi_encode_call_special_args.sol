pragma abicoder v2;

contract C {
	bool sideEffectRan = false;

	function fNoArgs() external {}
	function fArray(uint[] memory x) external {}
	function fUint(uint x, uint y) external returns (uint a, uint b) {}

	function fSignatureFromLiteralNoArgs() public pure returns (bytes memory) {
		return abi.encodeWithSignature("fNoArgs()");
	}
	function fPointerNoArgs() public view returns (bytes memory) {
		return abi.encodeCall(this.fNoArgs, ());
	}

	function fSignatureFromLiteralArray() public pure returns (bytes memory) {
		uint[] memory x;
		return abi.encodeWithSignature("fArray(uint256[])", x);
	}
	function fPointerArray() public view returns (bytes memory) {
		uint[] memory x;
		return abi.encodeCall(this.fArray, x);
	}

	function fSignatureFromLiteralUint() public pure returns (bytes memory) {
		return abi.encodeWithSignature("fUint(uint256,uint256)", 12, 13);
	}
	function fPointerUint() public view returns (bytes memory) {
		return abi.encodeCall(this.fUint, (12,13));
	}

	function assertConsistentSelectors() public view {
		assert(keccak256(fSignatureFromLiteralNoArgs()) == keccak256(fPointerNoArgs()));
		assert(keccak256(fSignatureFromLiteralArray()) == keccak256(fPointerArray()));
		assert(keccak256(fSignatureFromLiteralUint()) == keccak256(fPointerUint()));
	}
}
// ----
// assertConsistentSelectors() ->
// fSignatureFromLiteralNoArgs() -> 0x20, 0x04, 12200448252684243758085936796735499259670113115893304444050964496075123064832
// fPointerNoArgs() -> 0x20, 4, 12200448252684243758085936796735499259670113115893304444050964496075123064832
// fSignatureFromLiteralArray() -> 0x20, 0x44, 4612216551196396486909126966576324289294165774260092952932219511233230929920, 862718293348820473429344482784628181556388621521298319395315527974912, 0
// fPointerArray() -> 0x20, 0x44, 4612216551196396486909126966576324289294165774260092952932219511233230929920, 862718293348820473429344482784628181556388621521298319395315527974912, 0
// fPointerUint() -> 0x20, 0x44, 30372892641494467502622535050667754357470287521126424526399600764424271429632, 323519360005807677536004181044235568083645733070486869773243322990592, 350479306672958317330671196131255198757282877493027442254346933239808
// fSignatureFromLiteralUint() -> 0x20, 0x44, 30372892641494467502622535050667754357470287521126424526399600764424271429632, 323519360005807677536004181044235568083645733070486869773243322990592, 350479306672958317330671196131255198757282877493027442254346933239808
