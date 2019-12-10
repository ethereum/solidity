interface A {
    function utterance() external returns (bytes32);
}
contract B is A {
}
// ----
// TypeError: (69-88): Contract "B" should be marked as abstract.
