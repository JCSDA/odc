/*
 * (C) Copyright 1996-2012 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// \file WriterDispatchingIterator.h
///
/// @author Piotr Kuchta, June 2009

#ifndef WriterDispatchingIterator_H
#define WriterDispatchingIterator_H

#include "eckit/eckit.h"
#include "odblib/ColumnType.h"
#include "odblib/RowsIterator.h"
#include "odblib/TemplateParameters.h"
#include "odblib/Types.h"
#include "odblib/Writer.h"

namespace eckit { class PathName; }
namespace eckit { class DataHandle; }

namespace odb {

class HashTable;
class SQLIteratorSession;
class TemplateParameters;

//template <typename WRITE_ITERATOR = WriterBufferingIterator, typename OWNER = DispatchingWriter>
template <typename WRITE_ITERATOR, typename OWNER >
class WriterDispatchingIterator : public RowsWriterIterator
{
	typedef std::vector<double> Values;
	typedef std::map<Values,int> Values2IteratorIndex;
	typedef std::vector<WRITE_ITERATOR *> Iterators;
public:
	WriterDispatchingIterator (OWNER &owner, int maxOpenFiles, bool append = false);
	~WriterDispatchingIterator();

	int open();

	double* data();

    virtual int setColumn(size_t index, std::string name, ColumnType type) { NOTIMP; }
    virtual int setBitfieldColumn(size_t index, std::string name, ColumnType type, BitfieldDef b) { NOTIMP; }

	void missingValue(size_t i, double); 

	template <typename T> unsigned long pass1(T&, const T&);
	template <typename T> void verify(T&, const T&);
	unsigned long gatherStats(const double* values, unsigned long count);

	virtual int close();

	ColumnType columnType(size_t index);
    const std::string& columnName(size_t index) const;
    const std::string& codecName(size_t index) const;
	double columnMissingValue(size_t index);

	virtual MetaData& columns() { return columns_; }

	OWNER& owner() { return owner_; }

	void property(std::string key, std::string value);
	std::string property(std::string);

	std::vector<eckit::PathName> getFiles();
	TemplateParameters& templateParameters() { return templateParameters_; }

//protected:
	void writeHeader();

	int writeRow(const double* values, unsigned long count);

protected:
	bool next();

	/// Find iterator data should be dispatched to.
	WRITE_ITERATOR& dispatch(const double* values, unsigned long count);
    int createIterator(const Values& dispatchedValues, const std::string& fileName, const double* values, unsigned long count);

	std::string generateFileName(const double* values, unsigned long count);

	unsigned char *buffer_;
	OWNER& owner_;
	Writer<WRITE_ITERATOR> iteratorsOwner_;
	MetaData columns_;
	double* lastValues_;
	double* nextRow_;
	unsigned long long nrows_;
	std::string outputFileTemplate_;

	Properties properties_;

	std::vector<int> dispatchedIndexes_;
	Values2IteratorIndex values2iteratorIndex_;
	std::vector<unsigned long long> lastDispatch_;
	std::vector<std::string> iteratorIndex2fileName_;

	Values lastDispatchedValues_;
	int lastIndex_;
	bool initialized_;
	bool append_;

private:
// No copy allowed.
	WriterDispatchingIterator(const WriterDispatchingIterator&);
	WriterDispatchingIterator& operator=(const WriterDispatchingIterator&);

	void parseTemplateParameters();

	int refCount_;

	Iterators iterators_;
	std::vector<eckit::PathName> files_;

	TemplateParameters templateParameters_;
	int maxOpenFiles_;

	std::map<std::string,int> filesCreated_;
	std::vector<unsigned int> rowsOutputFileIndex_;

	friend class IteratorProxy<WriterDispatchingIterator<WRITE_ITERATOR,DispatchingWriter>, DispatchingWriter>;
};

} // namespace odb 

#include "odblib/WriterDispatchingIterator.cc"

#endif
