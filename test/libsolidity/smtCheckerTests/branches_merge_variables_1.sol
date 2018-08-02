pragma experimental SMTChecker;
// Branch does not touch variable a
contract C {
    function f(uint x) public pure {
        uint a = 3;
        if (x > 10) {
        }
        assert(a == 3);
    }
}
