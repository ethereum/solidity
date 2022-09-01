pragma abicoder v2;
// Example from https://github.com/ethereum/solidity/issues/12558
struct S {
    uint x;
}

contract C {
    S sStorage;
    constructor() {
        sStorage.x = 13;
    }

    function f() external returns (S[] memory) {
        S[] memory sMemory = new S[](1);

        sMemory[0] = sStorage;

        return sMemory;
    }
}
// ----
// f() -> 0x20, 1, 13
