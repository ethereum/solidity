pragma experimental ABIEncoderV2;
contract c {
    struct S { uint a ; }
    event E(S indexed);
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
