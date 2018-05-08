contract c1 {
    function f() payable payable {}
}
contract c2 {
    function f() view view {}
}
contract c3 {
    function f() pure pure {}
}
contract c4 {
    function f() pure view {}
}
contract c5 {
    function f() payable view {}
}
contract c6 {
    function f() pure payable {}
}
contract c7 {
    function f() pure constant {}
}
contract c8 {
    function f() view constant {}
}
// ----
// ParserError: (39-46): State mutability already specified as "payable".
// ParserError: (88-92): State mutability already specified as "view".
// ParserError: (134-138): State mutability already specified as "pure".
// ParserError: (180-184): State mutability already specified as "pure".
// ParserError: (229-233): State mutability already specified as "payable".
// ParserError: (275-282): State mutability already specified as "pure".
// ParserError: (324-332): State mutability already specified as "pure".
// ParserError: (374-382): State mutability already specified as "view".
