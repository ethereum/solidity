pragma experimental SMTChecker;
contract C {
    int[]data;

    function f() public {
        (data.pop)();
    }
}
// ----
// Warning 2529: (95-107): CHC: Empty array "pop" happens here.\nCounterexample:\ndata = []\n\nTransaction trace:\nC.constructor()\nState: data = []\nC.f()
