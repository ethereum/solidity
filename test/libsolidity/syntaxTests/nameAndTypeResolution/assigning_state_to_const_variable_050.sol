pragma experimental "v0.5.0";

contract C {
    address constant x = msg.sender;
}
// ----
// TypeError: (69-79): Initial value for constant variable has to be compile-time constant.
