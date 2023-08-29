pragma experimental solidity;

import { identity as id } from std.stub;

contract C
{
    fallback() external
    {
        id();
    }
}

// ====
// EVMVersion: >=constantinople
// ----
// Warning 2264: (std.stub:63-92): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning 2264: (0-29): Experimental features are turned on. Do not use experimental features on live deployments.
// Info 4164: (std.stub:94-117): Inferred type: () -> ()
// Info 4164: (std.stub:111-113): Inferred type: ()
// Info 4164: (40-48): Inferred type: () -> ()
// Info 4164: (90-135): Inferred type: () -> ()
// Info 4164: (98-100): Inferred type: ()
// Info 4164: (124-128): Inferred type: ()
// Info 4164: (124-126): Inferred type: () -> ()
