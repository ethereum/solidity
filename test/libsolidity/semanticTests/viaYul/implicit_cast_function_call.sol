// IRGeneratorForStatements::visit(FunctionCall const& _functionCall)
contract C {
    function f(uint b) public pure returns (uint x) {
        x = b;
    }
    function g() public pure returns (uint x) {
        uint8 a;
        assembly {
            a := 0x12345678
        }
        x = f(a);
    }
}
// ====
// compileViaYul: true
// ----
// g() -> 0x78
