error MyCustomError(uint, bool);

contract B {
    function f() mod1(MyCustomError) public { }
    modifier mod1(uint a) { if (a > 0) _; }
}

// ----
// TypeError 4649: (69-82): Invalid type for argument in modifier invocation. Invalid implicit conversion from error MyCustomError(uint256,bool) to uint256 requested.
