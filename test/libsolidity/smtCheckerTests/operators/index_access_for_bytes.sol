pragma experimental SMTChecker;

contract C {
    function f() public pure {
        bytes memory x = hex"00112233";
        assert(x[0] == 0x00);
        assert(x[1] == 0x11);
        assert(x.length == 3);
    }
}
// ----
// Warning 6328: (185-206): CHC: Assertion violation happens here.\nCounterexample:\n\nx = [0, 17, 34, 51]\n\nTransaction trace:\nC.constructor()\nC.f()
