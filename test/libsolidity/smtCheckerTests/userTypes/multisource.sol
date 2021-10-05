==== Source: A ====
type MyInt is int;
type MyAddress is address;
==== Source: B ====
import {MyInt, MyAddress as OurAddress} from "A";
contract A {
	function f(int x) internal pure returns(MyInt) { return MyInt.wrap(x); }
	function f(address x) internal pure returns(OurAddress) { return OurAddress.wrap(x); }

	function g() public pure {
		assert(MyInt.unwrap(f(int(5))) == 5);
		assert(MyInt.unwrap(f(int(5))) == 0); // should fail
		assert(OurAddress.unwrap(f(address(5))) == address(5));
		assert(OurAddress.unwrap(f(address(5))) == address(0)); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (B:296-332): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nA.constructor()\nA.g()\n    A.f(5) -- internal call\n    A.f(5) -- internal call
// Warning 6328: (B:409-463): CHC: Assertion violation happens here.\nCounterexample:\n\n\nTransaction trace:\nA.constructor()\nA.g()\n    A.f(5) -- internal call\n    A.f(5) -- internal call\n    A.f(0x05) -- internal call\n    A.f(0x05) -- internal call
