/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "targetver.h"
#include <iostream>
#include "LuceneHeaders.h"
#include "TermAttribute.h"

using namespace Lucene;

class LowerCaseNGramAnalyzer : public Analyzer {
public:
    LowerCaseNGramAnalyzer(int32_t minGram, int32_t maxGram) {
        this->minGram = minGram;
        this->maxGram = maxGram;
    }

    virtual ~LowerCaseNGramAnalyzer() {
    }

    LUCENE_CLASS(LowerCaseNGramAnalyzer);

protected:
    int32_t minGram;
    int32_t maxGram;

public:
    virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader) {
        return newLucene<LowerCaseFilter>(newLucene<NGramTokenizer>(reader, minGram, maxGram));
    }

    virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader) {
        TokenizerPtr tokenizer(boost::dynamic_pointer_cast<Tokenizer>(getPreviousTokenStream()));
        if (!tokenizer) {
            tokenizer = newLucene<NGramTokenizer>(reader, minGram, maxGram);
            setPreviousTokenStream(tokenizer);
        } else {
            tokenizer->reset(reader);
        }
        return newLucene<LowerCaseFilter>(tokenizer);
    }
};

static Collection<String> collectTerms(const AnalyzerPtr& analyzer, const String& fieldName, const String& text) {
    Collection<String> terms(Collection<String>::newInstance());
    TokenStreamPtr stream = analyzer->tokenStream(fieldName, newLucene<StringReader>(text));
    TermAttributePtr term = stream->addAttribute<TermAttribute>();

    while (stream->incrementToken()) {
        terms.add(term->term());
    }

    return terms;
}

static void printTerms(const String& title, Collection<String> terms) {
    std::wcout << title << L": ";
    for (int32_t i = 0; i < terms.size(); ++i) {
        if (i > 0) {
            std::wcout << L", ";
        }
        std::wcout << terms[i];
    }
    std::wcout << L"\n";
}

static QueryPtr buildNGramQuery(const String& fieldName, Collection<String> terms) {
    BooleanQueryPtr query = newLucene<BooleanQuery>();
    for (Collection<String>::iterator term = terms.begin(); term != terms.end(); ++term) {
        query->add(newLucene<TermQuery>(newLucene<Term>(fieldName, *term)), BooleanClause::MUST);
    }
    return query;
}

int main(int argc, char* argv[]) {
    try {
        const String fieldName = L"contents";
        const String inputText = L"AbCdEf";
        const String keyword = L"cDe";
        const int32_t minGram = 2;
        const int32_t maxGram = 3;

        AnalyzerPtr analyzer = newLucene<LowerCaseNGramAnalyzer>(minGram, maxGram);

        std::wcout << L"Input text: " << inputText << L"\n";
        std::wcout << L"Keyword: " << keyword << L"\n";
        std::wcout << L"NGram range: [" << minGram << L", " << maxGram << L"]\n";

        Collection<String> textTerms = collectTerms(analyzer, fieldName, inputText);
        Collection<String> keywordTerms = collectTerms(analyzer, fieldName, keyword);

        printTerms(L"Input ngram tokens", textTerms);
        printTerms(L"Keyword ngram tokens", keywordTerms);

        DirectoryPtr directory = newLucene<RAMDirectory>();
        IndexWriterPtr writer = newLucene<IndexWriter>(directory, analyzer, true, IndexWriter::MaxFieldLengthLIMITED);

        DocumentPtr doc = newLucene<Document>();
        doc->add(newLucene<Field>(fieldName, inputText, Field::STORE_YES, Field::INDEX_ANALYZED));
        writer->addDocument(doc);
        writer->close();

        SearcherPtr searcher = newLucene<IndexSearcher>(directory, true);
        QueryPtr query = buildNGramQuery(fieldName, keywordTerms);

        std::wcout << L"Query: " << query->toString(fieldName) << L"\n";

        TopDocsPtr topDocs = searcher->search(query, FilterPtr(), 10);
        std::wcout << L"Hits: " << topDocs->totalHits << L"\n";

        for (int32_t i = 0; i < topDocs->scoreDocs.size(); ++i) {
            DocumentPtr hitDoc = searcher->doc(topDocs->scoreDocs[i]->doc);
            std::wcout << L"Matched doc[" << i << L"]: " << hitDoc->get(fieldName) << L"\n";
        }

        searcher->close();
        directory->close();
    } catch (LuceneException& e) {
        std::wcout << L"Exception: " << e.getError() << L"\n";
        return 1;
    }

    return 0;
}
