contract C {
    modifier nonreentrant {
        assembly {
            if tload(0) { revert(0, 0) }
            tstore(0, 1)
        }
        _;
        assembly {
            tstore(0, 0)
        }
    }
    function f(bool simulateReentrancy) nonreentrant public {
        if (simulateReentrancy) {
            f(false);
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// f(bool): false ->
// f(bool): true -> FAILURE
