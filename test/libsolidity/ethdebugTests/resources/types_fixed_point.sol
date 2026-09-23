contract C {
    fixed128x18 signed;
    ufixed8x1 small;
}
// ----
// .resources.types | keys: ["t_fixed128x18","t_ufixed8x1"]
// .resources.types.t_fixed128x18: {"bits":128,"kind":"fixed","places":18}
// .resources.types.t_ufixed8x1: {"bits":8,"kind":"ufixed","places":1}
// .resources.pointers | keys: []
