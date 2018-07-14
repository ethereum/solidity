contract C {
    address constant x = msg.sender;
}
// ----
// TypeError: (38-48): Initial value for constant variable has to be compile-time constant.
