contract c {
    uint public a;
}
contract d {
    function g() public { c(0).a(); }
}
// ----
// Warning 2018: (51-84): Function state mutability can be restricted to view
