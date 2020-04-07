contract C {
  function f() public {
    (((((((((((,2),)),)),),))=4)));
  }
}
// ----
// TypeError: (53-54): Expression has to be an lvalue.
// TypeError: (67-68): Type int_const 4 is not implicitly convertible to expected type tuple(tuple(tuple(tuple(tuple(,int_const 2),),),),).
// TypeError: (44-68): Tuple component cannot be empty.
// TypeError: (43-69): Tuple component cannot be empty.
// TypeError: (42-70): Tuple component cannot be empty.
