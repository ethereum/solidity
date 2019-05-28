// IRGeneratorForStatements::visit(VariableDeclarationStatement const& _varDeclStatement)
contract C {
    function f() public pure returns (uint y) {
        uint8 a;
        assembly { a := 0x12345678 }
        uint z = a;
        y = z;
    }
}
// ====
// compileViaYul: true
// ----
// f() -> 0x78
