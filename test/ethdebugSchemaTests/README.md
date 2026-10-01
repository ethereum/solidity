# ETHDebug schema tests

These tests validate Solidity's ETHDebug output against a pinned checkout of the upstream `ethdebug/format` JSON Schemas.

The schema checkout is a git submodule at `test/ethdebugSchemaTests/ethdebug-format`. Initialize it with:

```bash
git submodule update --init test/ethdebugSchemaTests/ethdebug-format
```

Run the tests with:

```bash
test/ethdebugSchemaTests.py --solc-binary-path build/solc/solc
```

The runner discovers every test module in this directory and passes the
compiler path to the test cases.

The JSON inputs use `contentFile` entries to keep Solidity examples in regular
`.sol` fixture files under `sources/`. The tests expand those entries to
Standard JSON `content` before invoking `solc`.

To update the schema version, bump the submodule commit and rerun this suite.
