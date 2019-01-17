pragma experimental SMTChecker;
contract C {
    function f(bool x, bool y) public pure {
        uint a;
        if (x) {
            if (y) {
                a = 0;
            } else {
                a = 1;
            }
        } else {
            if (y) {
                a = 1;
            } else {
                a = 0;
            }
        }
        bool xor_x_y = (x && !y) || (!x && y);
        assert(!xor_x_y || a > 0);
    }
}
