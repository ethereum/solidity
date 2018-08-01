pragma experimental SMTChecker;
pragma experimental ABIEncoderV2;
contract C {
    struct A { uint a; uint b; }
    function f() public pure returns (A memory) {
        return A({ a: 1, b: 2 });
    }
}
// ----
// Warning: (32-65): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (150-158): Assertion checker does not yet support the type of this variable.
// Warning: (177-194): Assertion checker does not yet implement this expression.
