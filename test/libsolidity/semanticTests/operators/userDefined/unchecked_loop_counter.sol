type UncheckedCounter is uint;

using {
    add as +,
    lt as <
} for UncheckedCounter;

UncheckedCounter constant ONE = UncheckedCounter.wrap(1);

function add(UncheckedCounter x, UncheckedCounter y) pure returns (UncheckedCounter) {
    unchecked {
        return UncheckedCounter.wrap(UncheckedCounter.unwrap(x) + UncheckedCounter.unwrap(y));
    }
}

function lt(UncheckedCounter x, UncheckedCounter y) pure returns (bool) {
    return UncheckedCounter.unwrap(x) < UncheckedCounter.unwrap(y);
}

contract C {
    uint internalCounter = 12;

    function testCounter() public returns (uint) {
        for (UncheckedCounter i = UncheckedCounter.wrap(12); i < UncheckedCounter.wrap(20); i = i + ONE) {
            ++internalCounter;
        }
        return internalCounter;
    }
}
// ----
// testCounter() -> 20
