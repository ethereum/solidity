pragma experimental SMTChecker;
contract C {
    function f(bool x) public pure {
        if(x) {
            assert(x);
        } else {
            assert(!x);
        }
    }
}
