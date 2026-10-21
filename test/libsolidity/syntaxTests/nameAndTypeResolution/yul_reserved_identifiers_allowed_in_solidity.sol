// Identifiers reserved in Yul by EIP-8024 (swapn, dupn, exchange) can still be used as Solidity identifiers.
contract C0 {
    function swapn(uint dupn) public pure returns(uint exchange) {
        exchange = dupn;
    }
}

contract C1 {
    function dupn(uint exchange) public pure returns(uint swapn) {
        swapn = exchange;
    }
}

contract C2 {
    function exchange(uint swapn) public pure returns(uint dupn) {
        dupn = swapn;
    }
}

contract C3 {
    function foo() public pure returns(uint, uint, uint) {
        uint swapn = 0;
        uint dupn = 1;
        uint exchange = 2;

        return (swapn, dupn, exchange);
    }
}
// ----
