type T is uint;

contract C {
	function f(bytes memory data) public pure {
		T x = abi.decode(data, (T));
		T y = abi.decode(data, (T));
		assert(T.unwrap(x) == T.unwrap(y)); // should hold
		assert(T.unwrap(x) != T.unwrap(y)); // should fail
	}
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (192-226): CHC: Assertion violation happens here.
// Info 1391: CHC: 1 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
