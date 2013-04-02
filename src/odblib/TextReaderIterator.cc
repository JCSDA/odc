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
/// \file TextReaderIterator.cc
///
/// @author Piotr Kuchta, Oct 2010

#include "odblib/oda.h"

using namespace eclib;

typedef StringTools S;

namespace odb {

// TODO: allow user to specify the delimiter
string TextReaderIterator::defaultDelimiter = ",";

TextReaderIterator::TextReaderIterator(TextReader &owner)
: owner_(owner),
  columns_(0),
  lastValues_(0),
  nrows_(0),
  in_(0),
  newDataset_(false),
  noMore_(false),
  ownsF_(false),
  refCount_(0)
{
	in_ = &owner.stream();
	ASSERT(in_);

	parseHeader();
}

TextReaderIterator::TextReaderIterator(TextReader &owner, const PathName& pathName)
: owner_(owner),
  columns_(0),
  lastValues_(0),
  nrows_(0),
  in_(0),
  newDataset_(false),
  noMore_(false),
  ownsF_(false),
  refCount_(0)
{
	in_ = new std::ifstream(pathName.localPath());
	ASSERT(in_);
	ownsF_ = true;
	parseHeader();
}

void TextReaderIterator::parseHeader()
{
	string header;
	std::getline(*in_, header);
	vector<string> columns = S::split(defaultDelimiter, header);

	Log::debug() << "TextReaderIterator::parseHeader: defaultDelimiter: '" << defaultDelimiter << "'" << endl;
	Log::debug() << "TextReaderIterator::parseHeader: header: '" << header << "'" << endl;

	for (size_t i = 0; i < columns.size(); ++i)
	{
		vector<string> column = S::split(":", columns[i]);
		const string& columnName = column[0];
		const string& columnType = column[1];

		if (! S::startsWith(S::upper(columnType), "BITFIELD"))
		{
			columns_.addColumn<DataStream<SameByteOrder, DataHandle> >(columnName, columnType, true);
			Log::debug() << "TextReaderIterator::parseHeader: adding column " << columns_.size() << " '" << columnName << "' : " 
						<< columnType <<  endl;
		}
		else
		{
			const string& c(columns[i]);
			size_t leftBracket = c.find('[');
			size_t rightBracket = c.find(']');
			ASSERT(leftBracket != string::npos && rightBracket != string::npos);
			string s(c.substr(leftBracket + 1,  rightBracket - leftBracket - 1));
			//Log::debug() << "BITFIELD definition: '" << s << "'" << endl;

			odb::FieldNames names;
			odb::Sizes      sizes;
			vector<string> bs(S::split(";", s));
			for (size_t i = 0; i < bs.size(); ++i)
			{
				vector<string> v(S::split(":", bs[i]));
				ASSERT(v.size() == 2);
				names.push_back(v[0]);
				size_t size = atoi(v[1].c_str());
				ASSERT(size);
				sizes.push_back(size);
			}
			odb::BitfieldDef bd(make_pair(names, sizes));
			columns_.addBitfield<DataStream<SameByteOrder, DataHandle> >(columnName, bd);
			Log::debug() << "TextReaderIterator::parseHeader: adding BITFIELD " << columns_.size() << " '" << columnName << "' : " 
						<< /*bd <<*/ endl;
		}
	}
	initRowBuffer();
}

TextReaderIterator::~TextReaderIterator ()
{
	close();
	delete [] lastValues_;
}


bool TextReaderIterator::operator!=(const TextReaderIterator& other)
{
	ASSERT(&other == 0);
	return noMore_;
}

void TextReaderIterator::initRowBuffer()
{
	delete [] lastValues_;
	lastValues_ = new double [columns().size()];
	for(size_t i = 0; i < columns().size(); i++)
		lastValues_[i] = columns()[i]->missingValue(); 
}

bool TextReaderIterator::next()
{
	newDataset_ = false;
	if (noMore_)
		return false; 

	string line;
	std::getline(*in_, line);
	vector<string> values = S::split(defaultDelimiter, line);

	size_t nCols = values.size();
	if (nCols == 0)
		return ! (noMore_ = true);
	ASSERT(nCols == columns().size());

	for(size_t i = 0; i < nCols; ++i)
	{
		const string& v = values[i];
		if (v == "NULL")
			lastValues_[i] = columns_[i]->missingValue();
		else
			lastValues_[i] = StringTool::translate(v);
	}

	return nCols;
}

bool TextReaderIterator::isNewDataset() { return newDataset_; }

const double* TextReaderIterator::data() { return lastValues_; }

int TextReaderIterator::close()
{
	//if (ownsF_ && f) { f->close(); delete f; f = 0; }

	if (ownsF_ && in_)
	{
		delete in_;
		in_ = 0;
	}

	return 0;
}

} // namespace odb

