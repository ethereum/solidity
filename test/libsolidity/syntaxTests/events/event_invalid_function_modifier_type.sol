contract B {
    event MyCustomEvent(uint);
    function f() mod1(MyCustomEvent) public { }
    modifier mod1(uint a) { if (a > 0) _; }
}

// ----
// TypeError 4649: (66-79): Invalid type for argument in modifier invocation. Invalid implicit conversion from event MyCustomEvent(uint256) to uint256 requested.
