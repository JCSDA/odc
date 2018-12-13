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
/// \file ODA.cc
///
/// @author Piotr Kuchta, Feb 2009


#include <algorithm>
#include <iostream>
#include <sstream>
#include <errno.h>
#include <math.h>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/FileHandle.h"
#include "eckit/io/BufferedHandle.h"

#include "odc/Codec.h"
#include "odc/Column.h"
#include "odc/DataStream.h"
#include "odc/FixedSizeWriterIterator.h"
#include "odc/Header.h"
#include "odc/IteratorProxy.h"
#include "odc/MemoryBlock.h"
#include "odc/MetaData.h"
#include "odc/MetaDataReader.h"
#include "odc/MetaDataReaderIterator.h"
#include "odc/SelectIterator.h"
#include "eckit/sql/type/SQLBitfield.h"
#include "eckit/sql/expression/SQLExpression.h"
#include "eckit/sql/SQLSession.h"
#include "eckit/sql/SQLParser.h"
#include "eckit/sql/SQLSelect.h"
#include "eckit/sql/SQLTable.h"
#include "eckit/sql/type/SQLType.h"
#include "odc/WriterBufferingIterator.h"
#include "odc/Writer.h"

using namespace std;

namespace odc {

template <typename T>
MetaDataReader<T>::MetaDataReader()
: dataHandle_(0),
  deleteDataHandle_(true),
  path_(""),
  skipData_(true)
{} 

template <typename T>
MetaDataReader<T>::MetaDataReader(const eckit::PathName& path, bool skipData, bool buffered) :
    dataHandle_(buffered ? new eckit::BufferedHandle(path.fileHandle()) : path.fileHandle()),
    deleteDataHandle_(true),
    path_(path),
    skipData_(skipData)
{
	dataHandle_->openForRead();
} 

template <typename T>
MetaDataReader<T>::~MetaDataReader()
{
	if (dataHandle_ && deleteDataHandle_)
	{
		dataHandle_->close();
		delete dataHandle_;
	}
}

template <typename T>
typename MetaDataReader<T>::iterator* MetaDataReader<T>::createReadIterator(const eckit::PathName& pathName)
{
	return new T(*this, pathName, skipData_);
}

template <typename T>
typename MetaDataReader<T>::iterator MetaDataReader<T>::begin()
{
    T* it = new T(this->dataHandle(), skipData_);
    it->next();
	return iterator(it);
}

template <typename T>
const typename MetaDataReader<T>::iterator MetaDataReader<T>::end() { return iterator(0); }

} // namespace odc 