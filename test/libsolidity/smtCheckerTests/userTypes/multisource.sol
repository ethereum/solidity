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
// Warning 6328: (B:296-332): CHC: Assertion violation happens here.
// Warning 6328: (B:409-463): CHC: Assertion violation happens here.
// Info 1391: CHC: 2 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
