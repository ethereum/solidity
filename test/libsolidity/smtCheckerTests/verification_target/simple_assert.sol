pragma experimental SMTChecker;
contract C {
    function f(uint a) public pure { assert(a == 2); }
}
// ----
// Warning 4661: (82-96): Assertion violation happens here
