pragma experimental SMTChecker;

contract C {

    uint x;

    function reset_if_overflow() internal postinc {
        if (x < 10)
            return;
        x = 0;
    }

    modifier postinc() {
        if (x == 0) {
            return;
        }
        _;
        x = x + 1;
    }

    function test() public {
        if (x == 0) {
            reset_if_overflow();
            assert(x == 1); // should fail;
            assert(x == 0); // should hold;
            return;
        }
        if (x < 10) {
            uint oldx = x;
            reset_if_overflow();
            assert(oldx + 1 == x); // should hold;
            assert(oldx == x);     // should fail;
            return;
        }
        reset_if_overflow();
        assert(x == 1); // should hold;
        assert(x == 0); // should fail;
    }

    function set(uint _x) public {
        x = _x;
    }
}
// ----
// Warning 6328: (384-398): CHC: Assertion violation happens here.\nCounterexample:\nx = 0\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\ntest()
// Warning 6328: (635-652): CHC: Assertion violation happens here.\nCounterexample:\nx = 2\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nset(1)\nState: x = 1\ntest()
// Warning 6328: (781-795): CHC: Assertion violation happens here.\nCounterexample:\nx = 1\n\n\n\nTransaction trace:\nconstructor()\nState: x = 0\nset(10)\nState: x = 10\ntest()
