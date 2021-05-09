contract C {
    function f() public pure {
        bytes32 y = "abcdefghabcdefghabcdefghabcdefgh";
        bytes32 z = y;
        y &= "bcdefghabcdefghabcdefghabcdefgha";
        z &= "bcdefghabcdefghabcdefghabcdefgha";
        assert(y == "abcdefghabcdefghabcdefghabcdefgh"); // fails

        y |= "cdefghabcdefghabcdefghabcdefghab";
        z |= "cdefghabcdefghabcdefghabcdefghab";
        assert(y == "abcdefghabcdefghabcdefghabcd"); // fails

        y ^= "abcdefghabcdefghabcdefghabcdefgh";
        assert(y == z ^ "abcdefghabcdefghabcdefghabcdefgh");
    }
}
// ====
// SMTEngine: all
// ----
// Warning 6328: (229-276): CHC: Assertion violation happens here.\nCounterexample:\n\ny = 43595849750559157961410616371195012376776328331498503227444818324475146035296\nz = 43595849750559157961410616371195012376776328331498503227444818324475146035296\n\nTransaction trace:\nC.constructor()\nC.f()
// Warning 6328: (394-437): CHC: Assertion violation happens here.\nCounterexample:\n\ny = 44959890247538927454655645290332771782915717053340361485195502024921998844258\nz = 44959890247538927454655645290332771782915717053340361485195502024921998844258\n\nTransaction trace:\nC.constructor()\nC.f()
