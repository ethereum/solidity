abstract contract X { function test() private virtual returns (uint256); }
contract Y is X {
    uint256 public override test = 42;
}
contract T {
    constructor() public { new Y(); }
}
// ----
// TypeError: (97-130): Public state variables can only override functions with external visibility.
// TypeError: (22-72): "virtual" and "private" cannot be used together.
