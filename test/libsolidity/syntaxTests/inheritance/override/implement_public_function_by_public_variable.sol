contract X { function test() public returns (uint256); }
contract Y is X {
    uint256 public test = 42;
}
contract T {
    constructor() public { new Y(); }
}
// ----
// DeclarationError: (79-103): Identifier already declared.
