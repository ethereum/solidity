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
		assert(type(S1).typehash == 0x78a822935e38445215ba7404686b919cbbef5725cbf9231d92802f542d7456e0); // keccak256("S1(uint256 x)")
		assert(type(S2).typehash == 0x6c397ebd50462a81423e44830702ee1214cb9ab734bf173eb55f04238c9d398f); // keccak256("S2(uint256 x,address y)")
		assert(type(S3).typehash == 0xfa5685568fb2f15c09479ecbcfe9d0494743d804587d1966db67d5e62ea4344a); // keccak256("S3(uint256 x,address y,S2 third)S2(uint256 x,address y)")
		assert(type(S4).typehash == 0x17ed8da37c0446eeffef8cd38d116505b399b5644fdc3a59f8a68a68dd5d4178); // keccak256("S4(S3 one,S2 two)S2(uint256 x,address y)S3(uint256 x,address y,S2 third)")
		assert(type(S5).typehash == 0x5e52252fbbc0eda2d75f57c57d47fbec3bc6b215a9a3790c7f7ca44a36eb5185); // keccak256("S5(S2 two,S1 one,S3 three,S4[5] four)S1(uint256 x)S2(uint256 x,address y)S3(uint256 x,address y,S2 third)S4(S3 one,S2 two)")
	}
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
