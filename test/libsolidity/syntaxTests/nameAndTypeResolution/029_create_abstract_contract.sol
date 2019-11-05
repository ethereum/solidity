contract base { function foo() public; }
contract derived {
    base b;
    function foo() public { b = new base(); }
}
// ----
// TypeError: (0-40): Contract "base" should be marked as abstract.
