==== ExternalSource: ExtSource.sol=_external/external.sol ====
==== ExternalSource: /ExtSource.sol=_external/other_external.sol ====
import "ExtSource.sol";
import "/ExtSource.sol";
contract C {
    External _external;
    OtherExternal _otherExternal;
}
// ----
// constructor()
