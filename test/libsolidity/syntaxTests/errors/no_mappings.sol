error MyError(mapping(uint => uint));
contract C {
    error MyError2(mapping(uint => uint));
}
// ----
// TypeError 3448: (14-35): Type containing a (nested) mapping is not allowed as error parameter type.
