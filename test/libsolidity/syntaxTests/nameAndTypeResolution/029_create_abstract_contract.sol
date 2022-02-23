contract base { function foo() public virtual; }
contract derived {
    base b;
    function foo() public { b = new base(); }
}
// ----
// TypeError 3656: (0-48): Contract "base" should be marked as abstract.
