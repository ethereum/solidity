contract C {
    function f(uint a) public pure { assert(a == 2); }
}
// ====
// SMTEngine: all
// SMTIgnoreCex: yes
// ----
// Warning 6328: (50-64='assert(a == 2)'): CHC: Assertion violation happens here.
