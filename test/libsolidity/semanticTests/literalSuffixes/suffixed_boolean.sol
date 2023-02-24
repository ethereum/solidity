function not(bool a) pure suffix returns (bool) {
    return !a;
}

contract C {
    function notTrue() public pure returns (bool) {
        return true not;
    }

    function notFalse() public pure returns (bool) {
        return false not;
    }
}
// ----
// notTrue() -> false
// notFalse() -> true
