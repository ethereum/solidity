contract C {
    uint128 transient x;
    uint128 y;

    constructor() {
        x = 100;
        y = x;
    }

    function f() external view returns (uint128) {
        return y;
    }
}

// ====
// EVMVersion: >=cancun
// ----
// f() -> 100
