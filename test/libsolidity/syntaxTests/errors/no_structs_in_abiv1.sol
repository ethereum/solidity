pragma abicoder v1;
struct S {uint a;}
contract C {
    error MyError(S);
}
// ----
// TypeError 3061: (70-71): This type is only supported in ABI coder v2. Use "pragma abicoder v2;" to enable the feature.
