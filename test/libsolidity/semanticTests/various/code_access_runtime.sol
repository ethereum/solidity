contract D {
    uint256 x;

    constructor() {
        x = 7;
    }

    function f() public view returns (uint256) {
        return x;
    }
}

contract C {
    function test() public returns (uint256) {
        D d = new D();
        bytes32 hash;
        assembly { hash := extcodehash(d) }
        assert(hash == keccak256(type(D).runtimeCode));
        return 42;
    }
}

// ====
// EVMVersion: >=constantinople
// compileViaYul: also
// ----
// test() -> 42
// gas ir: 164781
// gas legacy: 118201
// gas legacyOptimized: 109663
