contract C {
    modifier m() {
        _;
    }

    modifier n() {
        string memory _ = "failed";
        _;
        revert(_);
    }

    function f() m() public returns (uint) {
        return 88;
    }

    function g() n() public returns (uint) {
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// f() -> 88
// g() -> FAILURE, hex"08c379a0", 0x20, 6, "failed"
