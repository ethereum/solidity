contract B {
    function f() mod1(true) public { }
    modifier mod1(uint a) { if (a > 0) _; }
}
// ----
// TypeError: (35-39): Invalid type for argument in modifier invocation. Invalid implicit conversion from bool to uint256 requested.
