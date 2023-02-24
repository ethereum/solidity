function length(bytes memory value) pure suffix returns (uint) {
    return value.length;
}

contract C {
    function empty() public pure returns (uint) {
        return "" length;
    }

    function short() public pure returns (uint) {
        return 'abcd' length;
    }

    function long() public pure returns (uint) {
        return 'abcdefghijklmnop abcdefghijklmnop' length;
    }

    function hex_() public pure returns (uint) {
        return hex'0123456789abcdef' length;
    }

    function unicode_() public pure returns (uint) {
        return unicode"😃" length;
    }
}
// ----
// empty() -> 0
// short() -> 4
// long() -> 33
// hex_() -> 8
// unicode_() -> 4
