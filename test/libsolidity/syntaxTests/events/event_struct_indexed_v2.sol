pragma experimental ABIEncoderV2;
contract c {
    struct S { uint a ; }
    event E(S indexed);
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (85-86): Reference types cannot be indexed.
