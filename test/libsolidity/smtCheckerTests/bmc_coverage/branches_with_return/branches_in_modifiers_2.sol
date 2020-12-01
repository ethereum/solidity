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
}
// ====
// SMTEngine: bmc
// ----
// Warning 4661: (384-398): BMC: Assertion violation happens here.
// Warning 4661: (635-652): BMC: Assertion violation happens here.
// Warning 4661: (781-795): BMC: Assertion violation happens here.
