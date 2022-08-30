==== Source: A.sol ====
import "B.sol";

library L {
    uint constant X = 1;
    uint constant Y = K.Y;
}

==== Source: B.sol ====
import "A.sol";

library K {
    uint constant X = L.X;
    uint constant Y = 2;
}

// ====
