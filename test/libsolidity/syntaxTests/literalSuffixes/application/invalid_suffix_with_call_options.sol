function suffix(uint value) pure suffix returns (uint) { return value; }

contract C {
    uint x = 1000 suffix{gas: 1};
}
// ----
// TypeError 2622: (100-119): Expected callable expression before call options.
