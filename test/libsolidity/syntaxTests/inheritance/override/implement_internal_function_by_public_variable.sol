contract X { function test() internal returns (uint256); }
contract Y is X {
    uint256 public test = 42;
}
contract T {
    constructor() public { new Y(); }
}
// ----
// DeclarationError: (81-105): Identifier already declared.
