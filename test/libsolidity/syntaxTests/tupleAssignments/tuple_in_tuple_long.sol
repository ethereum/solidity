contract C {
  function f() {
    (((((((((((,2),)),)),),))=4)));
  }
}
// ----
// SyntaxError: (15-69): No visibility specified. Did you intend to add "public"?
// TypeError: (46-47): Expression has to be an lvalue.
// TypeError: (60-61): Type int_const 4 is not implicitly convertible to expected type tuple(tuple(tuple(tuple(tuple(,int_const 2),),),),).
// TypeError: (37-61): Tuple component cannot be empty.
// TypeError: (36-62): Tuple component cannot be empty.
// TypeError: (35-63): Tuple component cannot be empty.
