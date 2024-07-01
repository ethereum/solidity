contract C {
function f(int[] calldata b) public pure {
    require(b.length == 3);
    require(b[0] == 0);
    require(b[1] == 1);
    require(b[2] == 2);
    assert(b[1:3][0] == 1); // should hold
    assert(b[1:3][1] == 1); // should fail
}
}
// ====
// SMTEngine: chc
// SMTIgnoreCex: no
// ----
// Warning 6328: (203-225): CHC: Assertion violation happens here.\nCounterexample:\n\nb = [0, 1, 2]\n\nTransaction trace:\nC.constructor()\nC.f([0, 1, 2])
// Info 1391: CHC: 6 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
