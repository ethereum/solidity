library L {
    function privateFunction(uint) private pure {}
}

using {L.privateFunction} for uint;

contract C {
    using {L.privateFunction} for uint;
}
// ----
// TypeError 6772: (73-90): Function "L.privateFunction" is private and therefore cannot be attached to a type outside of the library where it is defined.
// TypeError 6772: (127-144): Function "L.privateFunction" is private and therefore cannot be attached to a type outside of the library where it is defined.
