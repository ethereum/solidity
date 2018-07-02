contract base { function foo() public; }
contract derived {
    base b;
    function foo() public { b = new base(); }
}
// ----
// TypeError: (104-112): Trying to create an instance of an abstract contract.
