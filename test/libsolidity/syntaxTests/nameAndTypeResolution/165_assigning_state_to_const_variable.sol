contract C {
    address constant x = msg.sender;
}
// ----
// TypeError 8349: (38-48='msg.sender'): Initial value for constant variable has to be compile-time constant.
