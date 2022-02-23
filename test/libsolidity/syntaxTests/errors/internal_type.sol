error E1(function() internal);
error E2(S);

struct S {
    S[] ss;
}
// ----
// TypeError 3417: (9-29): Internal or recursive type is not allowed as error parameter type.
// TypeError 3417: (40-41): Internal or recursive type is not allowed as error parameter type.
