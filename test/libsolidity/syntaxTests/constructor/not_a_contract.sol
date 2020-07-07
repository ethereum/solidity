// This used to cause an internal error because of the visitation order.
contract Test {
    struct S { uint a; }
    function f() public {
       new S();
    }
}
// ----
// TypeError 5540: (147-152): Identifier is not a contract.
