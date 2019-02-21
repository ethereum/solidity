pragma solidity >=0.0;
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
