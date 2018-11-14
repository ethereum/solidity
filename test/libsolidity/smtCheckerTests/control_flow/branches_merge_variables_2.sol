pragma experimental SMTChecker;
// Positive branch touches variable a, but assertion should still hold.
contract C {
    function f(uint x) public pure {
        uint a = 3;
        if (x > 10) {
            a = 3;
        }
        assert(a == 3);
    }
}
