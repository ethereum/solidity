contract C {
    modifier onlyIf(address who, uint threshold) {
        require(msg.sender == who && msg.value >= threshold);
        _;
    }

    function withdraw() public payable onlyIf({threshold: 1 ether, who: msg.sender}) {}
}

// ----
// ParserError 6933: (190-191): Expected primary expression.
