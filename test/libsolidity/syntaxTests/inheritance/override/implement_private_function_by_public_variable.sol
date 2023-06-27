abstract contract X { function test() private virtual returns (uint256); }
contract Y is X {
    uint256 public override test = 42;
}
contract T {
    constructor() { new Y(); }
}
// ----
// TypeError 7792: (112-120): Public state variable has override specified but does not override anything.
// TypeError 3942: (22-72): "virtual" and "private" cannot be used together.
