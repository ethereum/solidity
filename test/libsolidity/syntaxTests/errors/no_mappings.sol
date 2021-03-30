error MyError(mapping(uint => uint));
contract C {
    error MyError2(mapping(uint => uint));
}
// ----
// TypeError 3448: (14-35): Type containing a (nested) mapping is not allowed as error parameter type.
// TypeError 3417: (14-35): Internal or recursive type is not allowed as error parameter type.
// TypeError 3448: (70-91): Type containing a (nested) mapping is not allowed as error parameter type.
// TypeError 3417: (70-91): Internal or recursive type is not allowed as error parameter type.
