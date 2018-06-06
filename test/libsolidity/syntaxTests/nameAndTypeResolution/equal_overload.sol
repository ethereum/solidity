contract C {
    function test(uint a) public returns (uint b) { }
    function test(uint a) external {}
}
// ----
// DeclarationError: (17-66): Function with same name and arguments defined twice.
// TypeError: (17-66): Overriding function visibility differs.
