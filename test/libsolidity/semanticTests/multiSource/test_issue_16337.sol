==== Source: Helper ====
library Helper {
	function getValue() internal pure returns (uint256) {
		return 42;
	}
}

==== Source: TestContract ====
import "Helper";
contract TestContract {
	function getValue() public pure returns (uint256) {
		return Helper.getValue();
	}
}
// ----
// getValue() -> 0x2a
