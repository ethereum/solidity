abstract contract X { function test() public returns (uint256); }
contract Y is X {
    uint256 public test = 42;
}
contract T {
    constructor() public { new Y(); }
}
// ----
// DeclarationError: (88-112): Identifier already declared.
