contract c {
    uint public a;
}
contract d {
    function g() public { c(address(0)).a(); }
}
// ----
// Warning 2018: (51-93): Function state mutability can be restricted to view
