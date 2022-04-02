struct S {
    S[] ss;
}

contract C {
    event E1(function() internal);
    event E2(S);
}
// ----
// TypeError 3417: (52-72='function() internal)'): Internal or recursive type is not allowed as event parameter type.
// TypeError 3417: (87-88='S'): Internal or recursive type is not allowed as event parameter type.
