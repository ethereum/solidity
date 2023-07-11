type Int is int128;

library L {
    function binaryOperator(Int, Int) private pure returns (Int) {}
    function unaryOperator(Int) private pure returns (Int) {}
}

using {L.binaryOperator as +} for Int global;
using {L.unaryOperator as -} for Int global;

contract C {
    using {L.binaryOperator as *} for Int;
    using {L.unaryOperator as ~} for Int;
}

library X {
    using {L.binaryOperator as *} for Int;
    using {L.unaryOperator as ~} for Int;
}
// ----
// TypeError 6772: (173-189): Function "L.binaryOperator" is private and therefore cannot be attached to a type outside of the library where it is defined.
// TypeError 7775: (173-189): Only pure free functions can be used to define operators.
// TypeError 6772: (219-234): Function "L.unaryOperator" is private and therefore cannot be attached to a type outside of the library where it is defined.
// TypeError 7775: (219-234): Only pure free functions can be used to define operators.
// TypeError 6772: (282-298): Function "L.binaryOperator" is private and therefore cannot be attached to a type outside of the library where it is defined.
// TypeError 3320: (282-298): Operators can only be defined in a global 'using for' directive.
// TypeError 7775: (282-298): Only pure free functions can be used to define operators.
// TypeError 6772: (325-340): Function "L.unaryOperator" is private and therefore cannot be attached to a type outside of the library where it is defined.
// TypeError 3320: (325-340): Operators can only be defined in a global 'using for' directive.
// TypeError 7775: (325-340): Only pure free functions can be used to define operators.
// TypeError 6772: (382-398): Function "L.binaryOperator" is private and therefore cannot be attached to a type outside of the library where it is defined.
// TypeError 3320: (382-398): Operators can only be defined in a global 'using for' directive.
// TypeError 7775: (382-398): Only pure free functions can be used to define operators.
// TypeError 6772: (425-440): Function "L.unaryOperator" is private and therefore cannot be attached to a type outside of the library where it is defined.
// TypeError 3320: (425-440): Operators can only be defined in a global 'using for' directive.
// TypeError 7775: (425-440): Only pure free functions can be used to define operators.
