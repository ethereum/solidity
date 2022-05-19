contract test {

	function uintMinA() public pure returns(bool) {

		uint8 uint8_min = type(uint8).min;
		require(uint8_min == 0);

		uint16 uint16_min = type(uint16).min;
		require(uint16_min == 0);

		uint24 uint24_min = type(uint24).min;
		require(uint24_min == 0);

		uint32 uint32_min = type(uint32).min;
		require(uint32_min == 0);

		uint40 uint40_min = type(uint40).min;
		require(uint40_min == 0);

		uint48 uint48_min = type(uint48).min;
		require(uint48_min == 0);

		uint56 uint56_min = type(uint56).min;
		require(uint56_min == 0);

		uint64 uint64_min = type(uint64).min;
		require(uint64_min == 0);

		return true;
	}

	function uintMinB() public pure returns(bool) {

		uint72 uint72_min = type(uint72).min;
		require(uint72_min == 0);

		uint80 uint80_min = type(uint80).min;
		require(uint80_min == 0);

		uint88 uint88_min = type(uint88).min;
		require(uint88_min == 0);

		uint96 uint96_min = type(uint96).min;
		require(uint96_min == 0);

		uint104 uint104_min = type(uint104).min;
		require(uint104_min == 0);

		uint112 uint112_min = type(uint112).min;
		require(uint112_min == 0);

		uint120 uint120_min = type(uint120).min;
		require(uint120_min == 0);

		uint128 uint128_min = type(uint128).min;
		require(uint128_min == 0);

		return true;
	}

	function uintMinC() public pure returns(bool) {

		uint136 uint136_min = type(uint136).min;
		require(uint136_min == 0);

		uint144 uint144_min = type(uint144).min;
		require(uint144_min == 0);

		uint152 uint152_min = type(uint152).min;
		require(uint152_min == 0);

		uint160 uint160_min = type(uint160).min;
		require(uint160_min == 0);

		uint168 uint168_min = type(uint168).min;
		require(uint168_min == 0);

		uint176 uint176_min = type(uint176).min;
		require(uint176_min == 0);

		uint184 uint184_min = type(uint184).min;
		require(uint184_min == 0);

		uint192 uint192_min = type(uint192).min;
		require(uint192_min == 0);

		return true;
	}

	function uintMinD() public pure returns(bool) {

		uint200 uint200_min = type(uint200).min;
		require(uint200_min == 0);

		uint208 uint208_min = type(uint208).min;
		require(uint208_min == 0);

		uint216 uint216_min = type(uint216).min;
		require(uint216_min == 0);

		uint224 uint224_min = type(uint224).min;
		require(uint224_min == 0);

		uint232 uint232_min = type(uint232).min;
		require(uint232_min == 0);

		uint240 uint240_min = type(uint240).min;
		require(uint240_min == 0);

		uint248 uint248_min = type(uint248).min;
		require(uint248_min == 0);

		uint256 uint256_min = type(uint256).min;
		require(uint256_min == 0);

		return true;
	}

	function uintMaxA() public pure returns (bool) {

		uint8 uint8_max = type(uint8).max;
		require(uint8_max == 2**8-1);

		uint16 uint16_max = type(uint16).max;
		require(uint16_max == 2**16-1);

		uint24 uint24_max = type(uint24).max;
		require(uint24_max == 2**24-1);

		uint32 uint32_max = type(uint32).max;
		require(uint32_max == 2**32-1);

		uint40 uint40_max = type(uint40).max;
		require(uint40_max == 2**40-1);

		uint48 uint48_max = type(uint48).max;
		require(uint48_max == 2**48-1);

		uint56 uint56_max = type(uint56).max;
		require(uint56_max == 2**56-1);

		uint64 uint64_max = type(uint64).max;
		require(uint64_max == 2**64-1);

		return true;
	}

	function uintMaxB() public pure returns (bool) {

		uint72 uint72_max = type(uint72).max;
		require(uint72_max == 2**72-1);

		uint80 uint80_max = type(uint80).max;
		require(uint80_max == 2**80-1);

		uint88 uint88_max = type(uint88).max;
		require(uint88_max == 2**88-1);

		uint96 uint96_max = type(uint96).max;
		require(uint96_max == 2**96-1);

		uint104 uint104_max = type(uint104).max;
		require(uint104_max == 2**104-1);

		uint112 uint112_max = type(uint112).max;
		require(uint112_max == 2**112-1);

		uint120 uint120_max = type(uint120).max;
		require(uint120_max == 2**120-1);

		uint128 uint128_max = type(uint128).max;
		require(uint128_max == 2**128-1);

		return true;
	}

	function uintMaxC() public pure returns (bool) {

		uint136 uint136_max = type(uint136).max;
		require(uint136_max == 2**136-1);

		uint144 uint144_max = type(uint144).max;
		require(uint144_max == 2**144-1);

		uint152 uint152_max = type(uint152).max;
		require(uint152_max == 2**152-1);

		uint160 uint160_max = type(uint160).max;
		require(uint160_max == 2**160-1);

		uint168 uint168_max = type(uint168).max;
		require(uint168_max == 2**168-1);

		uint176 uint176_max = type(uint176).max;
		require(uint176_max == 2**176-1);

		uint184 uint184_max = type(uint184).max;
		require(uint184_max == 2**184-1);

		uint192 uint192_max = type(uint192).max;
		require(uint192_max == 2**192-1);

		return true;
	}

	function uintMaxD() public pure returns(bool) {
		uint200 uint200_max = type(uint200).max;
		require(uint200_max == 2**200-1);

		uint208 uint208_max = type(uint208).max;
		require(uint208_max == 2**208-1);

		uint216 uint216_max = type(uint216).max;
		require(uint216_max == 2**216-1);

		uint224 uint224_max = type(uint224).max;
		require(uint224_max == 2**224-1);

		uint232 uint232_max = type(uint232).max;
		require(uint232_max == 2**232-1);

		uint240 uint240_max = type(uint240).max;
		require(uint240_max == 2**240-1);

		uint248 uint248_max = type(uint248).max;
		require(uint248_max == 2**248-1);

		uint256 uint256_max = type(uint256).max;
		require(uint256_max == 2**256-1);

		return true;
	}
}
// ====
// compileToEwasm: also
// ----
// uintMinA() -> true
// uintMinB() -> true
// uintMinC() -> true
// uintMinD() -> true
// uintMaxA() -> true
// uintMaxB() -> true
// uintMaxC() -> true
// uintMaxD() -> true
