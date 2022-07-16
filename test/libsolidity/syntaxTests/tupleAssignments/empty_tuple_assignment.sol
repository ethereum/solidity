function f() pure {
    ((, ())) = (1, 3);
}

function g() pure {
    ((, ((, ())))) = (1, 3);
}

function t() pure returns (int, int) {
    return (4, 5);
}

function h() pure {
    ((, ())) = t();
}

function ff() pure {
    ((((, ())) , )) = ((1, 2), 3);
}

function fg() pure {
    (((, ())) , ) = ((1, 2), 3);
}

// ----
// TypeError 5547: (28-30): Empty tuple on the left hand side.
// TypeError 7407: (35-41): Type tuple(int_const 1,int_const 3) is not implicitly convertible to expected type tuple(,tuple()).
// TypeError 5547: (78-80): Empty tuple on the left hand side.
// TypeError 7407: (87-93): Type tuple(int_const 1,int_const 3) is not implicitly convertible to expected type tuple(,tuple(,tuple())).
// TypeError 5547: (187-189): Empty tuple on the left hand side.
// TypeError 7407: (194-197): Type tuple(int256,int256) is not implicitly convertible to expected type tuple(,tuple()).
// TypeError 5547: (233-235): Empty tuple on the left hand side.
// TypeError 7407: (245-256): Type tuple(tuple(int_const 1,int_const 2),int_const 3) is not implicitly convertible to expected type tuple(tuple(,tuple()),).
// TypeError 5547: (291-293): Empty tuple on the left hand side.
// TypeError 7407: (302-313): Type tuple(tuple(int_const 1,int_const 2),int_const 3) is not implicitly convertible to expected type tuple(tuple(,tuple()),).
