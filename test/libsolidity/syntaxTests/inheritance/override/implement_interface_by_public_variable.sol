interface X { function test() external returns (uint256); }
contract Y is X {
    uint256 public test = 42;
}
contract T {
    constructor() public { new Y(); }
}
