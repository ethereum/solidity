function length(string memory value) pure suffix returns (uint) {
    return bytes(value).length;
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

    function unicode_() public pure returns (uint) {
        return unicode"😃" length;
    }
}
// ----
// empty() -> 0
// short() -> 4
// long() -> 33
// unicode_() -> 4
