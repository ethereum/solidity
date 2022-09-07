library   L { event E(uint8); }
contract B {
    constructor() {
        emit L.E(0);
    }
}
contract C is B {}

// ----
