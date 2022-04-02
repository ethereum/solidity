library C {
    receive() external payable {}
}
// ----
// DeclarationError 4549: (16-45='receive() external payable {}'): Libraries cannot have receive ether functions.
// TypeError 7708: (16-45='receive() external payable {}'): Library functions cannot be payable.
