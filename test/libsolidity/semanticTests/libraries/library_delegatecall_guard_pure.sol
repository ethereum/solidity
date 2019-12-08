library L {
    function f(uint256[] storage x) public pure returns (uint256) {
        return 23;
    }
}
contract C {
    uint256[] y;
    string x;
    constructor() public { y.push(42); }
    function f() public view returns (uint256) {
        return L.f(y);
    }
    function g() public returns (bool, uint256) {
        uint256 ys;
        assembly { ys := y_slot }
        (bool success, bytes memory data) = address(L).delegatecall(abi.encodeWithSelector(L.f.selector, ys));
        return (success, success ? abi.decode(data,(uint256)) : 0);
    }
    function h() public returns (bool, uint256) {
        uint256 ys;
        assembly { ys := y_slot }
        (bool success, bytes memory data) = address(L).call(abi.encodeWithSelector(L.f.selector, ys));
        return (success, success ? abi.decode(data,(uint256)) : 0);
    }
}
// ====
// EVMVersion: >homestead
// ----
// library: L
// f() -> 23
// g() -> true, 23
// h() -> true, 23
