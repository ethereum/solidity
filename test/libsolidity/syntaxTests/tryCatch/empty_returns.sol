contract C {
    function f() public {
        try this.f() returns () {

        } catch {

        }
    }
}
// ----
// ParserError: (69-70): Expected type name
