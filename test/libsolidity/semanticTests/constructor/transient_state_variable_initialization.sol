contract C {
    uint256 transient x;

    constructor() {
        x = 100;
    }

    function f() external view returns (uint256 x) {
        x;
    }
}

// ====
// EVMVersion: >=cancun
// ----
// f() -> 0
