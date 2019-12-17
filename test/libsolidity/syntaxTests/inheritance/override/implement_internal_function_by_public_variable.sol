abstract contract X { function test() internal virtual returns (uint256); }
contract Y is X {
    uint256 public override test = 42;
}
contract T {
    constructor() public { new Y(); }
}
// ----
// TypeError: (98-131): Public state variables can only override functions with external visibility.
