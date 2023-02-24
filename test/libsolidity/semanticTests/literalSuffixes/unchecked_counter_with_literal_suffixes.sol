type UncheckedCounter is uint;

using {
    add as +,
    lt as <
} for UncheckedCounter global;

function add(UncheckedCounter x, UncheckedCounter y) pure returns (UncheckedCounter) {
    unchecked {
        return UncheckedCounter.wrap(UncheckedCounter.unwrap(x) + UncheckedCounter.unwrap(y));
    }
}

function lt(UncheckedCounter x, UncheckedCounter y) pure returns (bool) {
    return UncheckedCounter.unwrap(x) < UncheckedCounter.unwrap(y);
}

function cycles(uint c) pure suffix returns (UncheckedCounter) {
    return UncheckedCounter.wrap(c);
}

contract C {
    uint public total = 0;

    function testCounter() public returns (uint) {
        for (UncheckedCounter i = 20 cycles; i < 23 cycles; i = i + 1 cycles)
            total += UncheckedCounter.unwrap(i);

        return total;
    }
}
// ----
// testCounter() -> 63
// total() -> 63
