contract C {
    function getOne() public payable nonFree returns (uint256 r) {
        return 1;
    }

    modifier nonFree {
        if (msg.value > 0) _;
    }
}
// ----
// getOne() -> 0
// getOne(), 1 wei -> 1
