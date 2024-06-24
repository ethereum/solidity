contract C {
    uint public transient pubt;
    uint internal transient it;
    uint private transient prvt;

    uint transient public tpub;
    uint transient internal ti;
    uint transient private tprv;
}
// ----
// UnimplementedFeatureError 1834: Transient storage variables are not supported.
