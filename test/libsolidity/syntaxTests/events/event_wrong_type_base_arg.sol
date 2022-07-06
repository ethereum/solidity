contract Base {
    event MyCustomEvent(uint);
    constructor(uint8) {}
}

contract Derived is Base(Base.MyCustomEvent) {}

// ----
// TypeError 9827: (101-119): Invalid type for argument in constructor call. Invalid implicit conversion from event MyCustomEvent(uint256) to uint8 requested.
