contract X { function test() internal returns (uint256); }
contract Y is X {
    uint256 public test = 42;
}
contract T {
    constructor() public { new Y(); }
}
// ----
// DeclarationError: (81-105): Identifier already declared.
// TypeError: (0-58): Contract "X" should be marked as abstract.
