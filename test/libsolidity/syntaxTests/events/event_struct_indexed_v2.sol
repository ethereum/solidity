pragma abicoder               v2;
contract c {
    struct S { uint a ; }
    event E(S indexed);
}
// ----
