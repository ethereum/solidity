contract C {
    int[]data;

    function f() public {
        (data.pop)();
    }
}
// ====
// SMTEngine: all
// ----
// Warning 2529: (63-75): CHC: Empty array "pop" happens here.\nCounterexample:\ndata = []\n\nTransaction trace:\nC.constructor()\nState: data = []\nC.f()
