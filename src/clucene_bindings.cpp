#include <iostream>
#include <v8.h>
#include <node.h>
//Thanks bnoordhuis and jerrysv from #node.js

#include <sstream>

#include <CLucene.h>
#include <CLucene/index/IndexModifier.h>
#include "Misc.h"
#include "repl_tchar.h"
#include "StringBuffer.h"

using namespace node;
using namespace v8;
using namespace std;
using namespace lucene::index;
using namespace lucene::analysis;
using namespace lucene::util;
using namespace lucene::store;
using namespace lucene::document;
using namespace lucene::search;
using namespace lucene::queryParser;

const static int CL_MAX_DIR = 220;

#define REQ_ARG_COUNT_AND_TYPE(I, TYPE) \
  if (args.Length() < (I + 1) ) { \
      return ThrowException(Exception::RangeError(String::New("A least " #I " arguments are required"))); \
  } else if (!args[I]->Is##TYPE()) { \
      return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a " #TYPE))); \
  }

#define REQ_FUN_ARG(I, VAR) \
  REQ_ARG_COUNT_AND_TYPE(I, Function) \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_STR_ARG(I) REQ_ARG_COUNT_AND_TYPE(I, String)
#define REQ_NUM_ARG(I) REQ_ARG_COUNT_AND_TYPE(I, Number)
#define REQ_OBJ_ARG(I) REQ_ARG_COUNT_AND_TYPE(I, Object)

#define REQ_OBJ_TYPE(OBJ, TYPE) \
  if (!OBJ->GetConstructorName()->Equals(String::New(#TYPE))) { \
      return ThrowException(Exception::TypeError(String::New("Expected a " #TYPE " type."))); \
  }

class LuceneDocument : public ObjectWrap {
public:
    static void Initialize(v8::Handle<v8::Object> target) {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        t->InstanceTemplate()->SetInternalFieldCount(1);

        NODE_SET_PROTOTYPE_METHOD(t, "addField", AddField);

        target->Set(String::NewSymbol("Document"), t->GetFunction());
    }

    void setDocument(Document* doc) { doc_ = doc; }
    Document* document() const { return doc_; }
protected:
    static Handle<Value> New(const Arguments& args) {
        HandleScope scope;

        LuceneDocument* newDoc = new LuceneDocument();
        newDoc->Wrap(args.This());

        return scope.Close(args.This());
    }

    // args:
    //   String* key
    //   String* value
    //   Integer flags
    static Handle<Value> AddField(const Arguments& args) {
        HandleScope scope;

        REQ_STR_ARG(0);
        REQ_STR_ARG(1);
        REQ_NUM_ARG(2);

        LuceneDocument* docWrapper = ObjectWrap::Unwrap<LuceneDocument>(args.This());

        TCHAR key[CL_MAX_DIR];
        STRCPY_AtoT(key, *String::Utf8Value(args[0]), CL_MAX_DIR);

        TCHAR value[CL_MAX_DIR];
        STRCPY_AtoT(value, *String::Utf8Value(args[1]), CL_MAX_DIR);

        try {
            Field* field = new Field(key, value, args[2]->Int32Value());
            docWrapper->document()->add(*field);
        } catch (CLuceneError& E) {
            return scope.Close(ThrowException(Exception::TypeError(String::New(E.what()))));
        } catch(...) {
            return scope.Close(ThrowException(Exception::Error(String::New("Unknown internal error while adding field"))));
        }
        
        return scope.Close(Undefined());
    }

    LuceneDocument() : ObjectWrap(), doc_(new Document) {
    }

    ~LuceneDocument() {
        delete doc_;
    }
private:
    Document* doc_;
};

class Lucene : public ObjectWrap {

    static Persistent<FunctionTemplate> s_ct;
    
private:
    int m_count;

public:

    static void Init(Handle<Object> target) {
        HandleScope scope;

        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        s_ct = Persistent<FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(String::NewSymbol("Lucene"));

        NODE_SET_PROTOTYPE_METHOD(s_ct, "addDocument", AddDocumentAsync);
        NODE_SET_PROTOTYPE_METHOD(s_ct, "deleteDocument", DeleteDocumentAsync);
        NODE_SET_PROTOTYPE_METHOD(s_ct, "search", SearchAsync);

        target->Set(String::NewSymbol("Lucene"), s_ct->GetFunction());
    }

    Lucene() : ObjectWrap(), m_count(0) {}

    ~Lucene() {}

    static Handle<Value> New(const Arguments& args) {
        HandleScope scope;
        Lucene* lucene = new Lucene();
        lucene->Wrap(args.This());
        return scope.Close(args.This());
    }
    
    struct index_baton_t {
        Lucene* lucene;         
        LuceneDocument* doc;
        v8::String::Utf8Value* docID;
        v8::String::Utf8Value* index;
        Persistent<Function> callback;
        uint64_t indexTime;
        uint64_t docsDeleted;
        std::string error;
    };
    
    // args:
    //   String* docID
    //   Document* doc
    //   String* indexPath
    static Handle<Value> AddDocumentAsync(const Arguments& args) {
        HandleScope scope;

        REQ_STR_ARG(0);
        REQ_OBJ_ARG(1);
        REQ_STR_ARG(2);
        REQ_FUN_ARG(3, callback);

        REQ_OBJ_TYPE(args.This(), Lucene);
        Lucene* lucene = ObjectWrap::Unwrap<Lucene>(args.This());

        index_baton_t* baton = new index_baton_t;
        baton->lucene = lucene;
        baton->docID = new v8::String::Utf8Value(args[0]);
        baton->doc = ObjectWrap::Unwrap<LuceneDocument>(args[1]->ToObject());
        baton->index = new v8::String::Utf8Value(args[2]);
        baton->callback = Persistent<Function>::New(callback);
        baton->error.clear();
        
        lucene->Ref();

        eio_custom(EIO_Index, EIO_PRI_DEFAULT, EIO_AfterIndex, baton);
        ev_ref(EV_DEFAULT_UC);

        return scope.Close(Undefined());
    }
        
    
    static int EIO_Index(eio_req* req) {
        index_baton_t* baton = static_cast<index_baton_t*>(req->data);

        lucene::analysis::standard::StandardAnalyzer an;
        IndexModifier* writer = 0;
        bool writerOpen = false;
        
        try {
          bool needsCreation = true;
          if (IndexReader::indexExists(*(*baton->index))) {
              if (IndexReader::isLocked(*(*baton->index))) {
                  IndexReader::unlock(*(*baton->index));
              }
              needsCreation = false;
          }
          writer = new IndexModifier(*(*baton->index), &an, needsCreation);
          writerOpen = true;
        
          // To bypass a possible exception (we have no idea what we will be indexing...)
          writer->setMaxFieldLength(0x7FFFFFFFL); // LUCENE_INT32_MAX_SHOULDBE
          // Turn this off to make indexing faster; we'll turn it on later before optimizing
          writer->setUseCompoundFile(false);
          uint64_t start = Misc::currentTimeMillis();

          // replace document._id if it's also set in the document itself
          TCHAR key[CL_MAX_DIR];
          STRCPY_AtoT(key, "_id", CL_MAX_DIR);
          TCHAR value[CL_MAX_DIR];
          STRCPY_AtoT(value, *(*baton->docID), CL_MAX_DIR);
          baton->doc->document()->removeFields(key);
          baton->doc->document()->add(*new Field(key, value, Field::STORE_YES|Field::INDEX_UNTOKENIZED));
          
          baton->docsDeleted = writer->deleteDocuments(new Term(key, value));
          writer->addDocument(baton->doc->document());
          
          // Make the index use as little files as possible, and optimize it
          writer->setUseCompoundFile(true);
          writer->optimize();

          baton->indexTime = (Misc::currentTimeMillis() - start);
        } catch (CLuceneError& E) {
          baton->error.assign(E.what());
        } catch(...) {
          baton->error = "Got an unknown exception";
        }
        
        // Close and clean up
        if (writerOpen == true) {
           writer->close();
        }       
        delete writer;
        //(*(*baton->index), &an, false);

        return 0;
    }

    static int EIO_AfterIndex(eio_req* req) {
        HandleScope scope;
        index_baton_t* baton = static_cast<index_baton_t*>(req->data);
        ev_unref(EV_DEFAULT_UC);
        baton->lucene->Unref();

        Handle<Value> argv[3];

        if (!baton->error.empty()) {
            argv[0] = v8::String::New(baton->error.c_str());
            argv[1] = Undefined();
            argv[2] = Undefined();
        }
        else {
            argv[0] = Undefined();
            argv[1] = v8::Integer::NewFromUnsigned((uint32_t)baton->indexTime);
            argv[2] = v8::Integer::NewFromUnsigned((uint32_t)baton->docsDeleted);
        }

        TryCatch tryCatch;

        baton->callback->Call(Context::GetCurrent()->Global(), 3, argv);

        if (tryCatch.HasCaught()) {
            FatalException(tryCatch);
        }

        baton->callback.Dispose();
        delete baton->index;
        delete baton;
        return 0;
    }
    
    
    struct indexdelete_baton_t {
        Lucene* lucene;         
        v8::String::Utf8Value* docID;
        v8::String::Utf8Value* index;
        Persistent<Function> callback;
        uint64_t indexTime;
        uint64_t docsDeleted;
        std::string error;
    };
    
    // args:
    //   String* docID
    //   String* indexPath
    static Handle<Value> DeleteDocumentAsync(const Arguments& args) {
        HandleScope scope;

        REQ_STR_ARG(0);
        REQ_STR_ARG(1);
        REQ_FUN_ARG(2, callback);

        REQ_OBJ_TYPE(args.This(), Lucene);
        Lucene* lucene = ObjectWrap::Unwrap<Lucene>(args.This());

        indexdelete_baton_t* baton = new indexdelete_baton_t;
        baton->lucene = lucene;
        baton->docID = new v8::String::Utf8Value(args[0]);
        baton->index = new v8::String::Utf8Value(args[1]);
        baton->callback = Persistent<Function>::New(callback);
        baton->error.clear();
        
        lucene->Ref();

        eio_custom(EIO_DeleteDocument, EIO_PRI_DEFAULT, EIO_AfterDeleteDocument, baton);
        ev_ref(EV_DEFAULT_UC);

        return scope.Close(Undefined());
    }
        
    
    static int EIO_DeleteDocument(eio_req* req) {
        indexdelete_baton_t* baton = static_cast<indexdelete_baton_t*>(req->data);

        lucene::analysis::standard::StandardAnalyzer an;
        IndexModifier* writer = 0;
        bool writerOpen = false;
        
        try {
          bool needsCreation = true;
          if (IndexReader::indexExists(*(*baton->index))) {
              if (IndexReader::isLocked(*(*baton->index))) {
                  IndexReader::unlock(*(*baton->index));
              }
              needsCreation = false;
          }
          writer = new IndexModifier(*(*baton->index), &an, needsCreation);
          writerOpen = true;
        
          // To bypass a possible exception (we have no idea what we will be indexing...)
          writer->setMaxFieldLength(0x7FFFFFFFL); // LUCENE_INT32_MAX_SHOULDBE
          // Turn this off to make indexing faster; we'll turn it on later before optimizing
          writer->setUseCompoundFile(false);
          uint64_t start = Misc::currentTimeMillis();
          
          TCHAR key[CL_MAX_DIR];
          STRCPY_AtoT(key, "_id", CL_MAX_DIR);
          TCHAR value[CL_MAX_DIR];
          STRCPY_AtoT(value, *(*baton->docID), CL_MAX_DIR);
          
          baton->docsDeleted = writer->deleteDocuments(new Term(key, value));
          
          // Make the index use as little files as possible, and optimize it
          writer->setUseCompoundFile(true);
          writer->optimize();

          baton->indexTime = (Misc::currentTimeMillis() - start);
        } catch (CLuceneError& E) {
          baton->error.assign(E.what());
        } catch(...) {
          baton->error = "Got an unknown exception";
        }
        
        // Close and clean up
        if (writerOpen == true) {
           writer->close();
        }       
        
        delete writer;
        //(*(*baton->index), &an, false);

        return 0;
    }

    static int EIO_AfterDeleteDocument(eio_req* req) {
        HandleScope scope;
        indexdelete_baton_t* baton = static_cast<indexdelete_baton_t*>(req->data);
        ev_unref(EV_DEFAULT_UC);
        baton->lucene->Unref();

        Handle<Value> argv[3];

        if (!baton->error.empty()) {
            argv[0] = v8::String::New(baton->error.c_str());
            argv[1] = Undefined();
            argv[2] = Undefined();
        }
        else {
            argv[0] = Undefined();
            argv[1] = v8::Integer::NewFromUnsigned((uint32_t)baton->indexTime);
            argv[2] = v8::Integer::NewFromUnsigned((uint32_t)baton->docsDeleted);
        }

        TryCatch tryCatch;

        baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);

        if (tryCatch.HasCaught()) {
            FatalException(tryCatch);
        }

        baton->callback.Dispose();
        delete baton->docID;
        delete baton->index;
        delete baton;
        return 0;
    }
    

    struct search_field
    {
        search_field(const std::string& key_, const std::string& value_) : key(key_), value(value_)
        { }
        std::string key;
        std::string value;
    };

    struct search_doc
    {
        float score;
        std::vector<search_field> fields;
    };

    struct search_baton_t
    {
        Lucene* lucene;
        std::string index;
        std::string search;
        std::vector<search_doc> docs;
        Persistent<Function> callback;
        std::string error;
    };

    static Handle<Value> SearchAsync(const Arguments& args) {
        HandleScope scope;

        REQ_STR_ARG(0);
        REQ_STR_ARG(1);
        REQ_FUN_ARG(2, callback);

        REQ_OBJ_TYPE(args.This(), Lucene);
        Lucene* lucene = ObjectWrap::Unwrap<Lucene>(args.This());

        search_baton_t* baton = new search_baton_t;
        baton->lucene = lucene;
        baton->index = *v8::String::Utf8Value(args[0]);
        baton->search = *v8::String::Utf8Value(args[1]);
        baton->callback = Persistent<Function>::New(callback);
        baton->error.clear();

        lucene->Ref();

        eio_custom(EIO_Search, EIO_PRI_DEFAULT, EIO_AfterSearch, baton);
        ev_ref(EV_DEFAULT_UC);

        return scope.Close(Undefined());
    }

    static int EIO_Search(eio_req* req) 
    {
        search_baton_t* baton = static_cast<search_baton_t*>(req->data);

        standard::StandardAnalyzer analyzer;
        IndexReader* reader = 0;
        try {
            reader = IndexReader::open(baton->index.c_str());
        } catch (CLuceneError& E) {
          baton->error.assign(E.what());
          return 0;
        } catch(...) {
          baton->error = "Got an unknown exception";
          return 0;
        }
        IndexReader* newreader = reader->reopen();
        if ( newreader != reader ) {
            delete reader;
            reader = newreader;
        }
        IndexSearcher s(reader);

        try {
            Query* q = QueryParser::parse(searchString, _T(""), &analyzer);
            TCHAR* searchString = STRDUP_AtoT(baton->search.c_str());
            Hits* hits = s.search(q);
            free(searchString);
            // Build the result array
            for (size_t i=0; i < hits->length(); i++) {
                Document& doc(hits->doc(i));
                // {"id":"ab34", "score":1.0}
                search_doc newDoc;
                newDoc.score = hits->score(i);

                Document::FieldsType* fields = const_cast<Document::FieldsType*>(doc.getFields());
                DocumentFieldEnumeration fieldEnum(fields->begin(), fields->end());
                while (fieldEnum.hasMoreElements()) {
                    Field* curField = fieldEnum.nextElement();

                    char* fieldName = STRDUP_TtoA(curField->name());
                    char* fieldValue = STRDUP_TtoA(curField->stringValue());

                    newDoc.fields.push_back(search_field(fieldName, fieldValue));

                    free(fieldName);
                    free(fieldValue);
                }
                baton->docs.push_back(newDoc);
            }
        } catch (CLuceneError& E) {
          baton->error.assign(E.what());
        } catch(...) {
          baton->error = "Got an unknown exception";
        }

        return 0;
    }

    static int EIO_AfterSearch(eio_req* req)
    {
        HandleScope scope;
        search_baton_t* baton = static_cast<search_baton_t*>(req->data);
        ev_unref(EV_DEFAULT_UC);
        baton->lucene->Unref();

        Handle<Value> argv[2];

        if (baton->error.empty()) {
            argv[0] = Null(); // Error arg, defaulting to no error

            Local<v8::Array> resultArray = v8::Array::New();
            for (int i = 0; i < baton->docs.size(); ++i) {
                search_doc& doc(baton->docs[i]);
                Local<Object> resultObject = Object::New();
                for (int j = 0; j < doc.fields.size(); ++j) {
                    search_field& field(doc.fields[i]);
                    resultObject->Set(String::New(field.key.c_str()), String::New(field.value.c_str()));
                }
                resultObject->Set(String::New("score"), Number::New(doc.score));
                resultArray->Set(i, resultObject);
            }

            argv[1] = resultArray;
        } else {
            argv[0] = String::New(baton->error.c_str());
            argv[1] = Null();
        }

        TryCatch tryCatch;

        baton->callback->Call(Context::GetCurrent()->Global(), 2, argv);

        if (tryCatch.HasCaught()) {
            FatalException(tryCatch);
        }
        
        baton->callback.Dispose();

        delete baton;

        return 0;
    }
};

Persistent<FunctionTemplate> Lucene::s_ct;

extern "C" void init(Handle<Object> target) {
    Lucene::Init(target);
    LuceneDocument::Initialize(target);
}

NODE_MODULE(clucene_bindings, init);
