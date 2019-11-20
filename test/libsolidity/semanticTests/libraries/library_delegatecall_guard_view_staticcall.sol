contract D {
    uint public x;
    constructor() public { x = 42; }
}
library L {
    function f(D d) public view returns (uint256) {
        return d.x();
    }
}
contract C {
    D d;
    constructor() public { d = new D(); }
    function f() public view returns (uint256) {
        return L.f(d);
    }
    function g() public returns (bool, uint256) {
        (bool success, bytes memory data) = address(L).delegatecall(abi.encodeWithSelector(L.f.selector, d));
        return (success, success ? abi.decode(data,(uint256)) : 0);
    }
    function h() public returns (bool, uint256) {
        (bool success, bytes memory data) = address(L).call(abi.encodeWithSelector(L.f.selector, d));
        return (success, success ? abi.decode(data,(uint256)) : 0);
    }
}
// ====
// EVMVersion: >homestead
// ----
// library: L
// f() -> 42
// g() -> true, 42
// h() -> true, 42
