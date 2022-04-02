interface A {
    function utterance() external returns (bytes32);
}
contract B is A {
}
// ----
// TypeError 3656: (69-88='contract B is A { }'): Contract "B" should be marked as abstract.
