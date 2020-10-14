// Removed to yield a warning, otherwise CI test fails with the expectation
// "no output requested"
//pragma solidity >=0.0;
pragma experimental SMTChecker;
contract test {
    function f(uint x) public pure {
		assert(x > 0);
    }
}