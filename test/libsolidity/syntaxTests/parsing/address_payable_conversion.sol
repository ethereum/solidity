contract C {
    function f() public pure {
        address payable a = address payable(this);
    }
}
// ----
//  ParserError: (80-87): Expected ';' but got 'payable'.
//  Warning: (87-93): Deleted tokens up to here.
