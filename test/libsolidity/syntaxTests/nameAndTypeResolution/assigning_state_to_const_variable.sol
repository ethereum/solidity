contract C {
    address constant x = msg.sender;
}
// ----
// Warning: (38-48): Initial value for constant variable has to be compile-time constant. This will fail to compile with the next breaking version change.
