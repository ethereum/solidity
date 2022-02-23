struct S {uint a;}
contract C {
    error MyError(S);
    error MyError2(S t);
}
// ----
