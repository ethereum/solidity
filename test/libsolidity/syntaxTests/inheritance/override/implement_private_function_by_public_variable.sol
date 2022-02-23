abstract contract X { function test() private virtual returns (uint256); }
contract Y is X {
    uint256 public override test = 42;
}
contract T {
    constructor() { new Y(); }
}
// ----
// TypeError 5225: (97-130): Public state variables can only override functions with external visibility.
// TypeError 3942: (22-72): "virtual" and "private" cannot be used together.
