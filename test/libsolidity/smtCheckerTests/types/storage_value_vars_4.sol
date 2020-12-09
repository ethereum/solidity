pragma experimental SMTChecker;
contract C
{
    function f() public view {
        assert(c > 0);
    }
    uint c;
}
// ----
// Warning 6328: (84-97): CHC: Assertion violation happens here.\nCounterexample:\nc = 0\n\n\n\nTransaction trace:\nconstructor()\nState: c = 0\nf()
