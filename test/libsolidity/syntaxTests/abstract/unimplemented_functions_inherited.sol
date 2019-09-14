abstract contract A {
    function a() public;
}
contract B is A {
}
// ----
// TypeError: (49-68): Contract "B" should be marked as abstract.
