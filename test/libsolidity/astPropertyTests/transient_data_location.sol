contract C {
    /// TransientDataLocation: storageLocation
    /// TransientVarName: name
    uint transient x;
    /// StorageDataLocation: storageLocation
    /// StorageVarName: name
    uint y;

}
// ====
// EVMVersion: >=cancun
// ----
// TransientDataLocation: transient
// TransientVarName: x
// StorageDataLocation: default
// StorageVarName: y
