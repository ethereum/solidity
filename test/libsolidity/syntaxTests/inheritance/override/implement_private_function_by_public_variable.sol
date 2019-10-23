contract X { function test() private returns (uint256); }
contract Y is X {
    uint256 public test = 42;
}
contract T {
    constructor() public { new Y(); }
}
// ----
// TypeError: (0-57): Contract "X" should be marked as abstract.
