var testCase = require('nodeunit').testCase,
    clucene = require('../clucene');

var document = null;
var index = './temp.index';

module.exports = testCase({
    test_new_document: function(assert) {
        assert.expect(1);
        clucene.newDocument(function(err, doc) {
            assert.equals(null, err, "err should be null");
            assert.notEquals(null, doc, "doc should not be null");
            document = doc;
            assert.done();
        });
    },
    test_add_field_to_document: function(assert) {
        assert.notEquals(null, document, "document should not be null");
        assert.doesNotThrow(function() {
          clucene.addField(doc, 'key', 'value', config, function(err) {
            assert.equals(null, err, "err should be null");
            assert.done();
          });
        }, "Should not throw an Error.");
    },
    test_add_document_to_index: function(assert) {
        assert.doesNotThrow(function() {clucene.addDocument(doc, index);}, "Should throw an Error. No params.");
        assert.done();
    }
});