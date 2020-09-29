pragma experimental SMTChecker;

contract C {
    function f(bool b1, bool b2) public pure {
        require(b1 || b2);
        uint c = b1 ? 3 : (b2 ? 2 : 1);
        assert(c > 1);
    }
}
// ----
// Warning 6838: (147-149): BMC: Condition is always true.
