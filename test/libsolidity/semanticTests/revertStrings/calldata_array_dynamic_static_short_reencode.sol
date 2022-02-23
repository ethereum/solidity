pragma abicoder               v2;
contract C {
    function f(uint256[][2][] calldata x) external returns (uint256) {
        return 42;
    }
    function g(uint256[][2][] calldata x) external returns (uint256) {
        return this.f(x);
    }
}
// ====
// EVMVersion: >=byzantium
// compileViaYul: also
// revertStrings: debug
// ----
// g(uint256[][2][]): 0x20, 0x01, 0x20, 0x00 -> FAILURE, hex"08c379a0", 0x20, 30, "Invalid calldata access offset"
