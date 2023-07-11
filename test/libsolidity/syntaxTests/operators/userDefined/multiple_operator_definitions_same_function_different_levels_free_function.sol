type Int is int;

function add(Int, Int) pure returns (Int) {}
function unsub(Int) pure returns (Int) {}

using {add as +, unsub as -} for Int global;

contract C {
    using {add as +, unsub as -} for Int;
}

library X {
    using {add as +, unsub as -} for Int;
}
// ----
// TypeError 3320: (176-179): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (186-191): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (233-236): Operators can only be defined in a global 'using for' directive.
// TypeError 3320: (243-248): Operators can only be defined in a global 'using for' directive.
