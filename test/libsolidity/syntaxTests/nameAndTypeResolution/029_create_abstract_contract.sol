contract base { function foo(); }
contract derived {
    base b;
    function foo() public { b = new base(); }
}
// ----
// TypeError: (97-105): Trying to create an instance of an abstract contract.
