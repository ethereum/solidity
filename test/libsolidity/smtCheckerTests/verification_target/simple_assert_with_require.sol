pragma experimental SMTChecker;
contract C {
    function f(uint a) public pure { require(a < 10); assert(a < 20); }
}
