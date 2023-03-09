contract C {
    function f() public pure {
        string memory x = "\x12\x34";
        bytes memory y = bytes(x);
        assert(y[0] == 0x12);
        assert(y[1] == 0x34);
        assert(y.length == 2);
    }
}
// ====
// SMTEngine: all
// ----
// Info 1391: CHC: 5 verification condition(s) proved safe! Enable the model checker option "show proved safe" to see all of them.
