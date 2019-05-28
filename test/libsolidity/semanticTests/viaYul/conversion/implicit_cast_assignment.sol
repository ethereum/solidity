// Tests IRGeneratorForStatements::visit(Assignment const& _assignment)
contract C {
    function f() public pure returns (uint16 x) {
        uint8 y;
        assembly {
            y := 0x12345678
        }
        x = y;
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 0x78
