contract C {
    function f() public returns (uint) {
    }
    function g(uint x, uint y) public returns (uint) {
    }
    function h() public payable returns (uint) {
    }
    function i(bytes32 b) public returns (bytes32) {
    }
    function j(bool b) public returns (bool) {
    }
    function k(bytes32 b) public returns (bytes32) {
    }
    function s() public returns (uint256[] memory) {
    }
    function t(uint) public pure {
    }
}
// ===
// compileViaYul: true
// ----
// f() -> 0
// g(uint256,uint256): 1, -2 -> 0
// h(), 1 ether -> 0
// i(bytes32), 1 ether: 2 -> FAILURE
// i(bytes32): 2 -> 0
// j(bool): true -> false
// k(bytes32): 0x31 -> 0x00
// s(): hex"4200ef" -> 0x20, 0
// t(uint256) -> FAILURE
