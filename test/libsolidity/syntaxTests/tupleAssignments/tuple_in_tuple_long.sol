contract C {
  function f() public {
    (((((((((((,2),)),)),),))=4)));
  }
}
// ----
// TypeError 4247: (53-54='2'): Expression has to be an lvalue.
// TypeError 7407: (67-68='4'): Type int_const 4 is not implicitly convertible to expected type tuple(tuple(tuple(tuple(tuple(,int_const 2),),),),).
// TypeError 6473: (44-68='((((((((,2),)),)),),))=4'): Tuple component cannot be empty.
// TypeError 6473: (43-69='(((((((((,2),)),)),),))=4)'): Tuple component cannot be empty.
// TypeError 6473: (42-70='((((((((((,2),)),)),),))=4))'): Tuple component cannot be empty.
