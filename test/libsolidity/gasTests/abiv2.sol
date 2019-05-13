pragma experimental ABIEncoderV2;

contract C {
    uint public a;
    uint[] public b;
    function f1(uint) public pure returns (uint) { }
    function f2(uint[] memory, string[] memory, uint16, address) public returns (uint[] memory, uint16[] memory) {}
    function f3(uint16[] memory, string[] memory, uint16, address) public returns (uint[] memory, uint16[] memory) {}
    function f4(uint32[] memory, string[12] memory, bytes[2][] memory, address) public returns (uint[] memory, uint16[] memory) {}
    function f5(address[] memory, string[] memory, bytes memory, address) public returns (uint[] memory, uint16[] memory) {}
    function f6(uint[30] memory, string[] memory, uint16, address) public returns (uint16[200] memory, uint16[] memory) {}
    function f7(uint[31] memory, string[20] memory, C, address) public returns (bytes[] memory, uint16[] memory) {}
    function f8(uint[32] memory, string[] memory, uint32, address) public returns (uint[] memory, uint16[] memory) {}
}
// ----
// creation:
//   codeDepositCost: 1122600
//   executionCost: 1167
//   totalCost: 1123767
// external:
//   a(): 530
//   b(uint256): infinite
//   f1(uint256): infinite
//   f2(uint256[],string[],uint16,address): infinite
//   f3(uint16[],string[],uint16,address): infinite
//   f4(uint32[],string[12],bytes[2][],address): infinite
//   f5(address[],string[],bytes,address): infinite
//   f6(uint256[30],string[],uint16,address): infinite
//   f7(uint256[31],string[20],address,address): infinite
//   f8(uint256[32],string[],uint32,address): infinite
