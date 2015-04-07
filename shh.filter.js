var chai = require('chai');
var web3 = require('../index');
var assert = chai.assert;
var FakeHttpProvider = require('./helpers/FakeHttpProvider');

var method = 'filter';

var tests = [{
    args: [{
        to: '0x47d33b27bb249a2dbab4c0612bf9caf4c1950855',
        topics: ['0x324f5435', '0x564b4566f3453']
    }],
    formattedArgs: [{
        to: '0x47d33b27bb249a2dbab4c0612bf9caf4c1950855',
        topics: ['0x324f5435', '0x564b4566f3453']
    }],
    result: '0xf',
    formattedResult: '0xf',
    call: 'shh_newFilter'
}];

describe('shh', function () {
    describe(method, function () {
        tests.forEach(function (test, index) {
            it('property test: ' + index, function () {
                
                // given
                var provider = new FakeHttpProvider();
                web3.setProvider(provider);
                provider.injectResult(test.result);
                provider.injectValidation(function (payload) {
                    assert.equal(payload.jsonrpc, '2.0');
                    assert.equal(payload.method, test.call);
                    assert.deepEqual(payload.params, test.formattedArgs);
                });

                // call
                web3.shh[method].apply(null, test.args);
                
            });
        });
    });
});


