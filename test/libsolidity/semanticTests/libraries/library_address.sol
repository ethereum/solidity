library L {
    function f(uint256 v) external pure returns (uint) {
        return v * v;
    }
    function g(uint256 v) external returns (uint) {
        return v * v;
    }
}
contract C {
    function addr() public view returns (bool) {
        return address(L) == address(0);
    }
    function g(uint256 v) public view returns (uint256) {
        return L.f(v);
    }
    function h(uint256 v) public returns (uint256) {
        (bool success, bytes memory result) = address(L).delegatecall(abi.encodeWithSignature("f(uint256)", v));
        assert(success);
        return abi.decode(result, (uint256));
    }
    function i(uint256 v) public returns (uint256) {
        (bool success, bytes memory result) = address(L).call(abi.encodeWithSignature("f(uint256)", v));
        assert(success);
        return abi.decode(result, (uint256));
    }
    function j(uint256 v) public returns (uint256) {
        (bool success, bytes memory result) = address(L).delegatecall(abi.encodeWithSignature("g(uint256)", v));
        assert(success);
        return abi.decode(result, (uint256));
    }
    function k(uint256 v) public returns (uint256) {
        (bool success, bytes memory result) = address(L).call(abi.encodeWithSignature("g(uint256)", v));
        assert(success);
        return abi.decode(result, (uint256));
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// library: L
// addr() -> false
// g(uint256): 1 -> 1
// g(uint256): 2 -> 4
// g(uint256): 4 -> 16
// h(uint256): 1 -> 1
// h(uint256): 2 -> 4
// h(uint256): 4 -> 16
// i(uint256): 1 -> 1
// i(uint256): 2 -> 4
// i(uint256): 4 -> 16
// j(uint256): 1 -> 1
// j(uint256): 2 -> 4
// j(uint256): 4 -> 16
// k(uint256): 1 -> FAILURE
// k(uint256): 2 -> FAILURE
// k(uint256): 4 -> FAILURE
