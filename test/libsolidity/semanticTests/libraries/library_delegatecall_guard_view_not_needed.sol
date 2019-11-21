library L {
    function f(uint256[] storage x) public view returns (uint256) {
        return 84;
    }
}
contract C {
    uint256[] y;
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
// f() -> 84
// g() -> true, 84
// h() -> true, 84
