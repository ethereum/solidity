pragma experimental SMTChecker;
contract C {
    function f(bool x, bool y) public pure {
        bool z = x || y;
        assert(!(x && y) || z);
    }
}
