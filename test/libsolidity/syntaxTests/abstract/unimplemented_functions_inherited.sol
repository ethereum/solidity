abstract contract A {
    function a() public virtual;
}
contract B is A {
}
// ----
// TypeError 3656: (57-76): Contract "B" should be marked as abstract.
