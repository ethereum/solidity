var chai = require('chai');
var assert = chai.assert;
var SandboxedModule = require('sandboxed-module');

SandboxedModule.registerBuiltInSourceTransformer('istanbul');
var HttpProvider = SandboxedModule.require('../lib/web3/httpprovider', {
    requires: {
        'xmlhttprequest': require('./helpers/FakeXMLHttpRequest')
    }
});

describe('lib/web3/httpprovider', function () {
    describe('send', function () {
        it('should send basic request', function () {
            var provider = new HttpProvider();
            var result = provider.send({});

            assert.equal(typeof result, 'object');
        });
    });

    describe('sendAsync', function () {
        it('should send basic async request', function (done) {
            var provider = new HttpProvider();

            provider.sendAsync({}, function (err, result) {
                assert.equal(typeof result, 'object');
                done();
            });
        }); 
    });
});

