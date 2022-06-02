function suffix(uint value) pure returns (uint) { return value; }

contract C {
    uint x = 1000 suffix{gas: 1};
}
// ----
// TypeError 2622: (93-112): Expected callable expression before call options.
