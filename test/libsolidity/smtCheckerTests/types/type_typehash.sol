interface I1 {
}

contract C {
    struct S1 {
        uint256 x;
    }

    struct S2 {
        uint256 x;
        address y;
    }

    struct S3 {
        uint256 x;
        I1 y;
        S2 third;
    }

    struct S4 {
        S3 one;
        S2 two;
    }

    struct S5 {
        S2 two;
        S1 one;
        S3 three;
        S4[5] four;
    }

	function f() public pure {
		assert(type(S1).typehash == keccak256("S1(uint256 x)"));
		assert(type(S2).typehash == keccak256("S2(uint256 x,address y)"));
		assert(type(S3).typehash == keccak256("S3(uint256 x,address y,S2 third)S2(uint256 x,address y)"));
        assert(type(S4).typehash == keccak256("S4(S3 one,S2 two)S2(uint256 x,address y)S3(uint256 x,address y,S2 third)"));
        assert(type(S5).typehash == keccak256("S5(S2 two,S1 one,S3 three,S4[5] four)S1(uint256 x)S2(uint256 x,address y)S3(uint256 x,address y,S2 third)S4(S3 one,S2 two)"));
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (386-441): CHC: Assertion violation happens here.
// Warning 6328: (445-510): CHC: Assertion violation happens here.
// Warning 6328: (514-611): CHC: Assertion violation happens here.
// Warning 6328: (621-735): CHC: Assertion violation happens here.
// Warning 6328: (745-909): CHC: Assertion violation happens here.
