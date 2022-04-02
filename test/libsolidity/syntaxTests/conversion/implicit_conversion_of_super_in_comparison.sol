contract D {}

contract C {
    C c;
    D d;

    function foo() public {
        // Current instance of the current contract vs super
        super != this;
        this != super;

        // Different instance of the current contract vs super
        super != c;
        c != super;

        // Instance of an unrelated contract vs super
        super != d;
        d != super;
    }
}
// ----
// TypeError 2271: (144-157='super != this'): Operator != not compatible with types type(contract super C) and contract C
// TypeError 2271: (167-180='this != super'): Operator != not compatible with types contract C and type(contract super C)
// TypeError 2271: (254-264='super != c'): Operator != not compatible with types type(contract super C) and contract C
// TypeError 2271: (274-284='c != super'): Operator != not compatible with types contract C and type(contract super C)
// TypeError 2271: (349-359='super != d'): Operator != not compatible with types type(contract super C) and contract D
// TypeError 2271: (369-379='d != super'): Operator != not compatible with types contract D and type(contract super C)
