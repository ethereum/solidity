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
// TypeError 2271: (144-157): Built-in binary operator != cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (167-180): Built-in binary operator != cannot be applied to types contract C and type(contract super C).
// TypeError 2271: (254-264): Built-in binary operator != cannot be applied to types type(contract super C) and contract C.
// TypeError 2271: (274-284): Built-in binary operator != cannot be applied to types contract C and type(contract super C).
// TypeError 2271: (349-359): Built-in binary operator != cannot be applied to types type(contract super C) and contract D.
// TypeError 2271: (369-379): Built-in binary operator != cannot be applied to types contract D and type(contract super C).
