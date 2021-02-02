contract C {
    error E();
    function f() public returns (uint, uint) {
        try this.f() {

        } catch E {

        }
    }
}
// ====
// EVMVersion: >=byzantium
// ----
// ParserError 2314: (117-118): Expected '(' but got '{'
