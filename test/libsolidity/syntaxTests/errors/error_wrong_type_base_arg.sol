error MyCustomError(uint, bool);

contract Base {
    constructor(uint8) {}
}

contract Derived is Base(MyCustomError) {}

// ----
// TypeError 9827: (104-117): Invalid type for argument in constructor call. Invalid implicit conversion from error MyCustomError(uint256,bool) to uint8 requested.
