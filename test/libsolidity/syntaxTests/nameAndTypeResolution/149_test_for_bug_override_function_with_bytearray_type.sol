contract Vehicle {
    function f(bytes calldata) external virtual returns (uint256 r) {r = 1;}
}
contract Bike is Vehicle {
    function f(bytes calldata) override external returns (uint256 r) {r = 42;}
}
// ----
// Warning: (23-95): Function state mutability can be restricted to pure
