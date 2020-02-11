contract C {
    /// Calling into non-existent account is successful (creates the account)
    function f() external returns(bool) {
        (bool success, ) = address(0x4242).call("");
        return success;
    }

    function h() external returns(bool) {
        (bool success, ) = address(0x4242).delegatecall("");
        return success;
    }
}

// ----
// f() -> 1
// h() -> 1
