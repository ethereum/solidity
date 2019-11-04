library C {
    receive() external payable {}
}
// ----
// TypeError: (16-45): Library functions cannot be payable.
// TypeError: (16-45): Libraries cannot have receive ether functions.
