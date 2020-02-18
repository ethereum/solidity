
		contract C {
			struct s1 { byte a; byte b; bytes10 c; bytes9 d; bytes10 e; }
			struct s2 { byte a; s1 inner; byte b; byte c; }
			byte x;
			s2 data;
			byte y;
			function test() public returns (bool) {
				x = 0x01;
				data.a = 0x02;
				data.inner.a = 0x03;
				data.inner.b = 0x04;
				data.inner.c = "1234567890";
				data.inner.d = "123456789";
				data.inner.e = "abcdefghij";
				data.b = 0x05;
				data.c = byte(0x06);
				y = 0x07;
				return x == 0x01 && data.a == 0x02 && data.inner.a == 0x03 && data.inner.b == 0x04 &&
					data.inner.c == "1234567890" && data.inner.d == "123456789" &&
					data.inner.e == "abcdefghij" && data.b == 0x05 && data.c == byte(0x06) && y == 0x07;
			}
		}
	
// ----
// test() -> 0x1

