type T is uint;

contract C {
	function f(bytes memory data) public pure {
		T x = abi.decode(data, (T));
		uint y = abi.decode(data, (uint));
		assert(T.unwrap(x) == y); // should hold
		assert(T.unwrap(x) != y); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (188-212): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
