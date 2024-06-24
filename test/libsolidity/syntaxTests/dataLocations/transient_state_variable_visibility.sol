contract C {
    uint public transient pubt;
    uint internal transient it;
    uint private transient prvt;

    uint transient public tpub;
    uint transient internal ti;
    uint transient private tprv;
}
// ----
// UnimplementedFeatureError 6715: (17-43): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (49-75): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (81-108): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (115-141): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (147-173): Transient storage is not yet implemented.
// UnimplementedFeatureError 6715: (179-206): Transient storage is not yet implemented.
