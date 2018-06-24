		contract C {
			struct S { uint a; uint b; }
			S x; S y;
			function f() public {
				S storage x_local = x;
				S storage y_local = y;
				S storage z_local = x;
				(x, y_local, x_local, z_local) = (y, x_local, y_local, y);
			}
		}
