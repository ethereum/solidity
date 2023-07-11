contract test {

	function intMinA() public pure returns (bool) {

		int8 int8_min = type(int8).min;
		require(int8_min == -2**7);

		int16 int16_min = type(int16).min;
		require(int16_min == -2**15);

		int24 int24_min = type(int24).min;
		require(int24_min == -2**23);

		int32 int32_min = type(int32).min;
		require(int32_min == -2**31);

		int40 int40_min = type(int40).min;
		require(int40_min == -2**39);

		int48 int48_min = type(int48).min;
		require(int48_min == -2**47);

		int56 int56_min = type(int56).min;
		require(int56_min == -2**55);

		int64 int64_min = type(int64).min;
		require(int64_min == -2**63);

		return true;
	}

	function intMinB() public pure returns(bool) {

		int72 int72_min = type(int72).min;
		require(int72_min == -2**71);

		int80 int80_min = type(int80).min;
		require(int80_min == -2**79);

		int88 int88_min = type(int88).min;
		require(int88_min == -2**87);

		int96 int96_min = type(int96).min;
		require(int96_min == -2**95);

		int104 int104_min = type(int104).min;
		require(int104_min == -2**103);

		int112 int112_min = type(int112).min;
		require(int112_min == -2**111);

		int120 int120_min = type(int120).min;
		require(int120_min == -2**119);

		int128 int128_min = type(int128).min;
		require(int128_min == -2**127);

		return true;
	}

	function intMinC() public pure returns (bool) {

		int136 int136_min = type(int136).min;
		require(int136_min == -2**135);

		int144 int144_min = type(int144).min;
		require(int144_min == -2**143);

		int152 int152_min = type(int152).min;
		require(int152_min == -2**151);

		int160 int160_min = type(int160).min;
		require(int160_min == -2**159);

		int168 int168_min = type(int168).min;
		require(int168_min == -2**167);

		int176 int176_min = type(int176).min;
		require(int176_min == -2**175);

		int184 int184_min = type(int184).min;
		require(int184_min == -2**183);

		int192 int192_min = type(int192).min;
		require(int192_min == -2**191);

		return true;
	}

	function intMinD() public pure returns(bool) {

		int200 int200_min = type(int200).min;
		require(int200_min == -2**199);

		int208 int208_min = type(int208).min;
		require(int208_min == -2**207);

		int216 int216_min = type(int216).min;
		require(int216_min == -2**215);

		int224 int224_min = type(int224).min;
		require(int224_min == -2**223);

		int232 int232_min = type(int232).min;
		require(int232_min == -2**231);

		int240 int240_min = type(int240).min;
		require(int240_min == -2**239);

		int248 int248_min = type(int248).min;
		require(int248_min == -2**247);

		int256 int256_min = type(int256).min;
		require(int256_min == -2**255);

		return true;
	}

	function intMaxA() public pure returns (bool) {

		int8 int8_max = type(int8).max;
		require(int8_max == 2**7-1);

		int16 int16_max = type(int16).max;
		require(int16_max == 2**15-1);

		int24 int24_max = type(int24).max;
		require(int24_max == 2**23-1);

		int32 int32_max = type(int32).max;
		require(int32_max == 2**31-1);

		int40 int40_max = type(int40).max;
		require(int40_max == 2**39-1);

		int48 int48_max = type(int48).max;
		require(int48_max == 2**47-1);

		int56 int56_max = type(int56).max;
		require(int56_max == 2**55-1);

		int64 int64_max = type(int64).max;
		require(int64_max == 2**63-1);

		return true;
	}

	function intMaxB() public pure returns(bool) {

		int72 int72_max = type(int72).max;
		require(int72_max == 2**71-1);

		int80 int80_max = type(int80).max;
		require(int80_max == 2**79-1);

		int88 int88_max = type(int88).max;
		require(int88_max == 2**87-1);

		int96 int96_max = type(int96).max;
		require(int96_max == 2**95-1);

		int104 int104_max = type(int104).max;
		require(int104_max == 2**103-1);

		int112 int112_max = type(int112).max;
		require(int112_max == 2**111-1);

		int120 int120_max = type(int120).max;
		require(int120_max == 2**119-1);

		int128 int128_max = type(int128).max;
		require(int128_max == 2**127-1);

		return true;
	}

	function intMaxC() public pure returns (bool) {

		int136 int136_max = type(int136).max;
		require(int136_max == 2**135-1);

		int144 int144_max = type(int144).max;
		require(int144_max == 2**143-1);

		int152 int152_max = type(int152).max;
		require(int152_max == 2**151-1);

		int160 int160_max = type(int160).max;
		require(int160_max == 2**159-1);

		int168 int168_max = type(int168).max;
		require(int168_max == 2**167-1);

		int176 int176_max = type(int176).max;
		require(int176_max == 2**175-1);

		int184 int184_max = type(int184).max;
		require(int184_max == 2**183-1);

		int192 int192_max = type(int192).max;
		require(int192_max == 2**191-1);

		return true;
	}

	function intMaxD() public pure returns(bool) {

		int200 int200_max = type(int200).max;
		require(int200_max == 2**199-1);

		int208 int208_max = type(int208).max;
		require(int208_max == 2**207-1);

		int216 int216_max = type(int216).max;
		require(int216_max == 2**215-1);

		int224 int224_max = type(int224).max;
		require(int224_max == 2**223-1);

		int232 int232_max = type(int232).max;
		require(int232_max == 2**231-1);

		int240 int240_max = type(int240).max;
		require(int240_max == 2**239-1);

		int248 int248_max = type(int248).max;
		require(int248_max == 2**247-1);

		int256 int256_max = type(int256).max;
		require(int256_max == 2**255-1);

		return true;
	}
}
// ----
// intMinA() -> true
// intMinB() -> true
// intMinC() -> true
// intMinD() -> true
// intMaxA() -> true
// intMaxB() -> true
// intMaxC() -> true
// intMaxD() -> true
