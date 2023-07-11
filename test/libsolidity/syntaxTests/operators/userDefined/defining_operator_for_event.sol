using {add as +} for C.Event global;

contract C {
    event Event();
}

function add(C.Event, C.Event) pure returns (C.Event) {}

// ----
// TypeError 5172: (21-28): Name has to refer to a user-defined type.
