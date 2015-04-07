var chai = require('chai');
var assert = chai.assert;
var SandboxedModule = require('sandboxed-module');

SandboxedModule.registerBuiltInSourceTransformer('istanbul');
var QtSyncProvider = SandboxedModule.require('../lib/web3/qtsync', {
    globals: {
        navigator: require('./helpers/FakeQtNavigator')
    }
});

describe('/lib/web3/qtsyncprovider', function () {
    describe('send', function () {
        it('should send basic request', function () {
            var provider = new QtSyncProvider();
            var result = provider.send({});

            assert.equal(typeof result, 'object');
        });
    });
});

