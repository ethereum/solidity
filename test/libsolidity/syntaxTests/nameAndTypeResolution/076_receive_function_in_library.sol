library C {
    receive() external payable {}
}
// ----
// TypeError 7708: (16-45): Library functions cannot be payable.
// TypeError 4549: (16-45): Libraries cannot have receive ether functions.
