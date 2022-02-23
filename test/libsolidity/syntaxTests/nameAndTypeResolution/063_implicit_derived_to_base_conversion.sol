contract A { }
contract B is A {
    function f() public { A a = B(address(1)); }
}
// ----
// Warning 2072: (59-62): Unused local variable.
// Warning 2018: (37-81): Function state mutability can be restricted to pure
