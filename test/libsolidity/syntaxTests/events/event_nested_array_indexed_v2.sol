pragma experimental ABIEncoderV2;
contract c {
    event E(uint[][] indexed);
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (59-67): Reference types cannot be indexed.
