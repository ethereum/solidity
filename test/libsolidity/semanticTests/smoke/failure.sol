contract C {
    function e() public {
        revert("Transaction failed.");
    }
}
// ====
// EVMVersion: >homestead
// ----
// _() -> FAILURE
// e() -> FAILURE, hex"08c379a0", 0x20, 19, "Transaction failed."
